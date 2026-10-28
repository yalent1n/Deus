#pragma once
#include "base.h"

namespace rw
{
class HttpServerConnection;

struct http_path_part
{
    static constexpr s16 LABEL = 0;
    static constexpr s16 CAPTURE = 1;
    static constexpr s16 CAPTURE_WILDCARD = 2;

    const char* Text;
    s16 Type;
    s16 Offset;
};

FB_NETWORK_API bool HTTPPartUri(const char* uri, eastl::vector<http_path_part>& parts);

struct HttpRequest
{
    HttpRequest()
        : method((HttpVerb) -1)
        , path(NULL)
        , query()
        , headers()
        , body()
        , content(NULL)
        , connection(NULL)
    {}

    HttpVerb method;
    char* path;
    DbObject query;
    DbObject params;
    eastl::vector<HttpHeader> headers;
    DbObject body;
    char* content;
    StreamSize content_length;
    HttpServerConnection* connection;
    bool is_nakedquery;
};

class HttpResponse
{
public:
    HttpResponse() = default;

    HttpStatus status(HttpStatus e) {  
        m_Status = e;
        return m_Status;
    }

    HttpResponse& body(DbObject&& body) {
        m_Body = std::move(body);
        return *this;
    }

    HttpResponse& content(const char* content, StreamSize contentLength = -1) {
        m_Content = content;
        m_ContentLength = contentLength == -1 ? strlen(content) : contentLength;
        return *this;
    }

    HttpResponse& header(const char* key, const char* text)
    {
        m_Headers.push_back({key, text});
        return *this;
    }

    HttpStatus m_Status;
    DbObject m_Body;
    const char* m_Content;
    StreamSize m_ContentLength;
    eastl::vector<HttpHeader> m_Headers;
};

typedef FastDelegate<HttpStatus(const HttpRequest&, HttpResponse&)> HttpAction;

struct http_path_param
{
    const char* id;
    const char* chars;
};

// Examples:
// "/services/(name)/(*idata)" -> szervizkezelo endpoint (router)
// "/api/(version)/(endpoint)/(*idata)" -> api kezelo endpoint (router)
// "/records/(id)" -> visszaadja a record adatot
// "/records/(id)/metadata" -> visszaadja a record metaadatait (size, piece infos)
class HttpRouter
{
public:
    FB_NETWORK_API HttpRouter(const char* uri);

public:
    const HttpAction& getAction(int verb) const
        { return m_Actions[verb]; }
    
    void setAction(int verb, HttpAction&& action) 
        { m_Actions[verb] = action; }

    MIME getFormType() const
        { return m_FormType; }

    void setFormType(MIME form)
        { m_FormType = form; }

    CodecMethod getFormCodec()
        { return m_FormCodec; }

    void setFormCodec(CodecMethod codec)
        { m_FormCodec = codec; }

    FB_NETWORK_API bool doesURIMatch(const char* uri);
    FB_NETWORK_API bool doesURIMatchAndCapture(const char* uri, eastl::vector<http_path_param>& captures);

private:
    const char* m_URI;
    eastl::vector<http_path_part> m_Parts;
    HttpAction m_Actions[HTTP_VERB_COUNT];
    MIME m_FormType;
    CodecMethod m_FormCodec;
};
}