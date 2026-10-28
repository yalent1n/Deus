#include <Engine.Network/http/server.h>
#include <Engine.Base/Misc/Codec.h>

namespace rw
{
enum HTTP_CONNECTION_STATE
{
    http_STATUS_reading_verb,

    http_STATUS_reading_uri,
    http_STATUS_reading_uri_query,

    http_STATUS_done_uri,
    http_STATUS_done,

    http_HEADERS_reading_header_key,
    http_HEADERS_reading_header_value,
    http_HEADERS_done,

    http_CONTENT_reading_payload,
    http_CONTENT_done,

    http_REQUEST_passed
};

HttpServerConnection::HttpServerConnection(HttpServer* srv)
    : Buffer()
    , m_Request()

    , m_Srv(srv)
    , m_BytesProgressed(0)
    , m_State(http_STATUS_reading_verb)

    , m_PathOffset(0)
    
    , m_CurrentHeaderKeyOffset(0)
    , m_CurrentHeaderValueOffset(0)
    
    , m_ContentOffset(0)
    , m_ContentLength(0)
{}

void HttpServerConnection::onDataAvailable()
{
    ScopedRef PieceBuf = dequeueRead();
    size_t PieceSize = PieceBuf->size64();
    Buffer.write(PieceBuf->bytes(), PieceSize);

    char* Cursor = (char*) Buffer.bytes() + m_BytesProgressed;

    size_t amtBytesProgressed = 0;
    if(m_State < http_HEADERS_reading_header_key)
    {
        amtBytesProgressed = ProgressInitial(Cursor, PieceSize);
        m_BytesProgressed += amtBytesProgressed;
        Cursor = (char*) Buffer.bytes() + m_BytesProgressed;

        if(m_State == http_HEADERS_reading_header_key)
        {
            size_t amtBytesTillPieceEnd = PieceSize - amtBytesProgressed;
            m_BytesProgressed += ProgressData(Cursor, amtBytesTillPieceEnd);
        }
    }
    else
    {
        amtBytesProgressed = ProgressData(Cursor, PieceSize);
        m_BytesProgressed += amtBytesProgressed;
        Cursor = (char*) Buffer.bytes() + m_BytesProgressed;
    }

    if(m_State == http_CONTENT_done)
    {
        if(m_ContentLength)
        {
            const char* payload = (const char*) Buffer.bytes() + m_ContentOffset;
            MIME mime = CMime::fromString(FetchHeaderText("content-type"));
            const char* encoding = FetchHeaderText("content-encoding");
            EASSERT(isnil(encoding), "TODO: handle content-encoding");
            ProcessPayload(payload, mime, encoding);
            m_ContentLength = 0;
        }

        m_Request.connection = this;
        m_Srv->enqueue(m_Request);
        m_State = http_REQUEST_passed;
        return;
    }

    if(m_State == http_REQUEST_passed)
    {
        EnqueueResponse(HTTP_NOT_EXTENDED);
    }
}

void HttpServerConnection::onConnectionEnd()
{
}

void HttpServerConnection::onDisconnect()
{
}

void HttpServerConnection::EnqueueResponse(HttpStatus status, const HttpServerConnection::SendAdditionalData* addon)
{
    StringBuilder<> res;
    res << "HTTP/1.1 ";
    tostring((int)status, res);
    res << ' ';
    const char* statusString = CHttpStatus::toString(status);
    res << statusString;
    res << "\r\n";

    const char* Content = NULL;
    const char* ContentType = NULL;
    const char* ContentEncoding = NULL;
    int ContentLength = 0;
    if(addon) {
        Content = addon->Content;
        ContentType = CMime::toString(addon->ContentType);
        ContentLength = addon->ContentLength;
        
        if(addon->ContentEncoding != CodecMethod_None)
        {
            ContentEncoding = codecMethodToString(addon->ContentEncoding);

            MemorySharedBuffer wrap(Content, ContentLength);
            SharedBuffer* encodedContent = Codec::encode(addon->ContentEncoding, &wrap, CodecSegmentSize);

            Content = (const char*) (encodedContent->bytes());
            ContentLength = (int) encodedContent->size();
        }

        if(!addon->Headers.empty())
        {
            for(const HttpHeader& hdr : addon->Headers)
            {
                res << hdr.Name << ": " << hdr.Text << "\r\n";
            }
        }
    }

    res << "Server: jinks\r\n";
    res << "Connection: close\r\n";
    res << "Content-Length: ";
    tostring(ContentLength, res);
    res << "\r\n";
    
    if(Content)
    {
        if(!ContentType)
            ContentType = CMime::toString(Mime_Plain);

        res << "Content-Type: " << ContentType << "\r\n";

        if(ContentEncoding)
            res << "Content-Encoding: " << ContentEncoding << "\r\n";
    }

    res << "\r\n";

    if(Content)
    {
        res.appendChars(Content, ContentLength);
    }


    int responseSize = res.size();
    const char* responseText = res.detach();
    enqueueWrite(new MemorySharedBuffer(responseText, responseSize, YES));
}

const char* HttpServerConnection::FetchHeaderText(const char* Key) const
{
    const HttpHeader* hdr = algorithm::linear_search(m_Request.headers.begin(), m_Request.headers.end(), Key, HttpHeaderEquals());
    if(hdr != m_Request.headers.end())
    {
        return hdr->Text;
    }

    return NULL;
}

void HttpServerConnection::ProcessPayload(const char* Payload, MIME ContentType, const char* ContentEncoding)
{
    if(ContentType == Mime_Json)
    {
        bool Parsed = m_Request.body.fromJsonString(Payload, m_ContentLength);
        if(!Parsed)
        {
            EnqueueResponse(HTTP_NOT_ACCEPTABLE);
            return;
        }
    }
    else if(ContentType == Mime_Url)
    {
        DbObjectBuilder builder;

        for(QueryStringIterator it(Payload, m_ContentLength); it.more();)
        {
            const QueryParameter& param = it.moveNext();
            builder << param.key << param.value;
        }

        m_Request.body = builder.done();
    }
    else
    {
        m_Request.content = (char*) HeapAllocGlobal(m_ContentLength + 1);
        memcpy(m_Request.content, Payload, m_ContentLength);
        m_Request.content[m_ContentLength] = 0;
        m_Request.content_length = m_ContentLength;
    }
}

size_t HttpServerConnection::ProgressData(char* Cursor, size_t PieceSize)
{
    char* begin = Cursor;
    char* end = Cursor + PieceSize;

    char* hKeyStr = NULL;
    char* hValueStr = NULL;

_DISPATCH2:
    switch(m_State)
    {
        default:
        case http_CONTENT_done:
            break;

        case http_CONTENT_reading_payload:
        {
            m_ContentBytesProgessed += PieceSize;
            if(m_ContentBytesProgessed >= m_ContentLength)
            {
                m_State = http_CONTENT_done;
                goto _DISPATCH2;
            }

            return PieceSize;
        }

        case http_HEADERS_done:
        {
            int amtBytesProgessed = (int)(Cursor - begin);
            m_ContentOffset = m_BytesProgressed + amtBytesProgessed;
            m_ContentBytesProgessed = (int) PieceSize - amtBytesProgessed;

            m_State = (m_ContentBytesProgessed == m_ContentLength) ? http_CONTENT_done : http_CONTENT_reading_payload;
            break;
        }

        case http_HEADERS_reading_header_key:
        {
            if(m_CurrentHeaderKeyOffset)
            {
                hKeyStr = (char*) Buffer.bytes() + m_CurrentHeaderKeyOffset;
            }
            else
            {
                hKeyStr = Cursor;
                m_CurrentHeaderKeyOffset = m_BytesProgressed + (int) (Cursor - begin);
            }

            if(hKeyStr[0] == '\r' && hKeyStr[1] == '\n')
            {
                hKeyStr = NULL;
                m_CurrentHeaderKeyOffset = 0;
                hValueStr = NULL;
                m_CurrentHeaderValueOffset = 0;

                Cursor += 2;
                m_State = http_HEADERS_done;
                goto _DISPATCH2;
            }

            while(Cursor[0] != ':' && Cursor < end)
                Cursor++;
            if(Cursor == end)
                return PieceSize;
            *Cursor++ = 0;

            m_State = http_HEADERS_reading_header_value;
            goto _DISPATCH2;
        }
        case http_HEADERS_reading_header_value:
        {
            if(m_CurrentHeaderKeyOffset)
                hKeyStr = (char*) Buffer.bytes() + m_CurrentHeaderKeyOffset;
            
            if(m_CurrentHeaderValueOffset)
            {
                hValueStr = (char*) Buffer.bytes() + m_CurrentHeaderValueOffset;
            }
            else
            {
                while(isspace(Cursor[0]) && Cursor[0] && Cursor < end)
                    Cursor++;
                if(Cursor == end)
                    return PieceSize;

                hValueStr = Cursor;
                m_CurrentHeaderValueOffset = m_BytesProgressed + (int) (Cursor - begin);
            }

            while(Cursor[0] != '\r' && Cursor < end)
                Cursor++;
            if(Cursor == end)
                return PieceSize;
            *Cursor++ = 0;

            EASSERT(Cursor[0] == '\n', "Invalid end of header line");
            Cursor++;

            if(equalsIgnoreCase("content-length", hKeyStr))
            {
                bool Converted = tointeger(hValueStr, m_ContentLength);
                EASSERT(Converted, "Invalid content-length header");
            }

            if(!isnil(hKeyStr))
            {
                HttpHeader hdr = { safedup(hKeyStr), safedup(hValueStr) };
                m_Request.headers.push_back(hdr);
            }

            hKeyStr = NULL;
            m_CurrentHeaderKeyOffset = 0;
            hValueStr = NULL;
            m_CurrentHeaderValueOffset = 0;

            m_State = http_HEADERS_reading_header_key;
            goto _DISPATCH2;
        }
    }

    return (size_t) (Cursor - begin);
}

size_t HttpServerConnection::ProgressInitial(char* Cursor, size_t PieceSize)
{
    char* begin = Cursor;
    char* end = Cursor + PieceSize;
    char* queryString = NULL;

_DISPATCH:
    switch(m_State)
    {
        default:
        case http_STATUS_done:
            m_State = http_HEADERS_reading_header_key;
            break;

        case http_STATUS_reading_verb:
        {
            char* TmpVerb = Cursor;
    
            while(Cursor[0] != ' ')
                Cursor++;
            *Cursor++ = 0;

            m_Request.method = (HttpVerb) -1;
            for(int i = 0; i < HTTP_VERB_COUNT; i++)
            {
                if(equalsIgnoreCase(s_HttpVerbs[i], TmpVerb))
                {
                    m_Request.method = (HttpVerb) i;
                    break;
                }
            }

            EASSERT(m_Request.method != (HttpVerb) -1, "Invalid HTTP verb");
            m_State = http_STATUS_reading_uri;
            goto _DISPATCH;
        }
        case http_STATUS_reading_uri:
        {            
            char* pathStr = Cursor;
            if(m_PathOffset)
            {
                pathStr = (char*) Buffer.bytes() + m_PathOffset;
            }
            else
            {
                m_PathOffset = (int) (pathStr - begin);
            }

            while(Cursor[0] != ' ')
            {
                if(Cursor >= end)
                    return PieceSize;

                if(Cursor[0] == '?')
                {
                    Cursor[0] = 0;
                    queryString = Cursor + 1;
                }
                
                Cursor++;
            }
            *Cursor++ = 0;

            m_Request.path = (char*) safedup(pathStr);
            m_State = queryString ? http_STATUS_reading_uri_query : http_STATUS_done_uri;
            goto _DISPATCH;
        }
        case http_STATUS_reading_uri_query:
        {
            if(queryString[1])
            {
                m_Request.is_nakedquery = false;
                DbObjectBuilder queryBuilder;
                for(QueryStringIterator it(queryString, ((Cursor - 1) - queryString)); it.more();)
                {
                    QueryParameter param = it.moveNext();
                    queryBuilder.append(param.key, param.value);
                }
                m_Request.query = queryBuilder.done();
            }
            else
            {
                m_Request.is_nakedquery = true;
            }
            m_State = http_STATUS_done_uri;
            goto _DISPATCH;
        }
        case http_STATUS_done_uri:
        {
            while(Cursor[0] != '\r')
                Cursor++;
            
            if(Cursor >= end)
                return PieceSize;

            Cursor += 2;
            m_State = http_STATUS_done;
            goto _DISPATCH;
        }
    }

    return (size_t) (Cursor - begin);
}

HttpServer::HttpServer(u16 Port, WorkerThreadPool* ThreadPool, bool Https)
    : TcpServer(Port, ThreadPool, Https, NET_HTTP_MTU)
    , m_Routers(true)
{}

HttpServer::~HttpServer()
{}

TcpServerConnection* HttpServer::newConnection()
{
    return new HttpServerConnection(this);
}

void HttpServer::onConnect(TcpServerConnection* connection)
{
    //ELOGF("Http connection %p", connection);
}

void HttpServer::onDisconnect(TcpServerConnection* connection)
{
    //ELOGF("Http disconnection %p", connection);
}

void HttpServer::update()
{
    TcpServer::update();

    while(!m_RequestQueue.empty())
    {
        HttpRequest** req = &m_RequestQueue.front();
        complete(**req);
        m_RequestQueue.erase(req);
    }
}

HttpRouter* HttpServer::addRouter(const char* path)
{
    m_Routers.ensure(1);
    HttpRouter* router = m_Routers.begin() + m_Routers.size();
    m_Routers.set_size(m_Routers.size() + 1);
    new (router) HttpRouter(path);
    return router;
}

//void deleteRouter(HttpRouter* router);

HttpRouter* HttpServer::fetchRouterFromUri(const char* uri)
{
    for(HttpRouter& router : m_Routers)
    {
        if(router.doesURIMatch(uri))
        {
            return &router;
        }
    }

    return nullptr;
}

HttpRouter* HttpServer::fetchRouterAndCaptureFromUri(const char* uri, eastl::vector<http_path_param>& result)
{
    for(HttpRouter& router : m_Routers)
    {
        if(router.doesURIMatchAndCapture(uri, result))
        {
            return &router;
        }
    }

    return nullptr;
}

void HttpServer::enqueue(HttpRequest& req)
{
    m_RequestQueue.push_back(&req);
}

void HttpServer::error(HttpStatus status, HttpRequest& req)
{
    switch(status)
    {
        case HTTP_BAD_GATEWAY:
            req.connection->EnqueueResponse(HTTP_BAD_GATEWAY);
            break;
        case HTTP_METHOD_NOT_ALLOWED:
            req.connection->EnqueueResponse(HTTP_METHOD_NOT_ALLOWED);
            break;
        case HTTP_UNSUPPORTED_MEDIA_TYPE:
            req.connection->EnqueueResponse(HTTP_UNSUPPORTED_MEDIA_TYPE);
            break;
    }
}

void HttpServer::complete(HttpRequest& req)
{
    eastl::vector<http_path_param> captures;
    HttpRouter* router = fetchRouterAndCaptureFromUri(req.path, captures);
    if(!router)
    {
        error(HTTP_BAD_GATEWAY, req);
        return;
    }

    const HttpAction& action = router->getAction(req.method);
    if(!action)
    {
        error(HTTP_METHOD_NOT_ALLOWED, req);
        return;
    }

    DbObjectBuilder params;
    for(const http_path_param& param : captures)
    {
        params << param.id << param.chars;
    }
    req.params = params.done();

    HttpResponse res {};
    HttpStatus status = action(req, res);

    const char* payload = res.m_Content;
    StreamSize payloadLength = res.m_ContentLength;
    if(!res.m_Body.isNullOrEmpty())
    {
        switch(router->getFormType())
        {
            default:
                error(HTTP_UNSUPPORTED_MEDIA_TYPE, req);
                return;

            case Mime_Json:
            {
                StringBuilder<> json;

                JsonConversionParams convParams;
                convParams.indentChildren = false;
                convParams.indentRoots = false;
                res.m_Body.toJsonString(json, convParams);

                payload = json.detach();
                payloadLength = strlen(payload);
                break;
            }

            case Mime_Url:
            {
                eastl::vector<QueryParameter> params;
                for(DbObjectElementIterator it(res.m_Body); it.more();)
                {
                    DbObjectElement e = it.next();
                    
                    StringBuilder<> value;
                    e.getValueSafe(value);
                    params.push_back({e.name(), value.detach()});
                }

                StringBuilder<> query;
                urlEncode(params.begin(), params.size(), query);

                payload = query.detach();
                payloadLength = strlen(payload);
                break;
            }

            case Mime_Fson:
            {
                payload = (const char*) res.m_Body.data();
                payloadLength = res.m_Body.dataSize();
                break;
            }
        }
    }

    HttpServerConnection::SendAdditionalData addon {};
    addon.Content = payload;
    addon.ContentType = router->getFormType();
    addon.ContentEncoding = router->getFormCodec();
    addon.ContentLength = payloadLength;
    addon.Headers = std::move(res.m_Headers);

    req.connection->EnqueueResponse(status, &addon);
}
}