#include <Engine.Network/HttpSession.h>
#include <Engine.Base/Buffer/Adapter.h>
#include <Engine.Base/Database/DbObjectBuilder.h>
#include <malloc.h>

#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

namespace rw
{
namespace httpUtil
{
u32 getNumber(HINTERNET hRequest, DWORD query)
{
	wchar_t str[32]{0};
	DWORD num = 0;
	DWORD len = sizeof(str);

	if(!WinHttpQueryHeaders(hRequest,
	   query,
	   WINHTTP_HEADER_NAME_BY_INDEX,
	   str,
	   &len,
	   WINHTTP_NO_HEADER_INDEX))
	{
		DWORD err = GetLastError();
		if(err == ERROR_WINHTTP_HEADER_NOT_FOUND || query == WINHTTP_QUERY_CONTENT_LENGTH)
		{
			BOOL bRes = WinHttpQueryDataAvailable(hRequest, &num);
			EASSERT(bRes, "Failed to query content length");
			return u32(num);
		}

		return 0;
	}

	return num;
}
}

const char* verbName(HttpVerb method)
{
	switch(method)
	{
	case HTTP_VERB_GET: return "GET";
	case HTTP_VERB_POST: return "POST";
	case HTTP_VERB_PUT: return "PUT";
	case HTTP_VERB_HEAD: return "HEAD";
	case HTTP_VERB_DELETE: return "DELETE";
	}

	return "";
}

HttpVerb methodVerb(const char* method)
{
	if(equalsIgnoreCase(method, "GET"))
		return HTTP_VERB_GET;
	if(equalsIgnoreCase(method, "POST"))
		return HTTP_VERB_POST;
	if(equalsIgnoreCase(method, "PUT"))
		return HTTP_VERB_PUT;
	if(equalsIgnoreCase(method, "HEAD"))
		return HTTP_VERB_HEAD;
	if(equalsIgnoreCase(method, "DELETE"))
		return HTTP_VERB_DELETE;

	return (HttpVerb) -1;
}

void urlDecode(void* buffer, u32 len)
{
	char* buf = static_cast<char*>(buffer);
	
	if(isnil(buf) || len == 0)
		return;

	u32 i = 0, o = 0;
	
	while(i < len)
	{
		const char c = buf[i];
		if(c == '%' && (i + 2) < len)
		{
			const char chars[2] = { buf[i+1], buf[i+2] };
			fromhexstr(chars, 2, buf + o, 1);
			++o;
			i += 3;
		}
		else
		{
			buf[o++] = c;
			++i;
		}
	}

	if(o != i)
		buf[o] = '\0';
}

void urlEncode(QueryParameter* params, u32 count, StringEditor& builder)
{
	if(!params || !count)
		return;

	for(u32 i = 0; i < count; i++)
	{
		const QueryParameter& param = params[i];
		builder.appendChars(param.key);
		builder.appendChar('=');

		const char* rawvalue = param.value;
		size_t rawvaluelen = strlen(rawvalue);
		for(size_t j = 0; j < rawvaluelen; j++)
		{
			char c = rawvalue[j];
			if(isalchar(c) || isalnum(c))
				builder.appendChar(c);
			else
			{
				builder.appendChar('%');
				tohexstr(&c, 1, builder, false);
			}
		}

		if(i != count - 1)
			builder.appendChar('&');
	}
}

QueryStringIterator::QueryStringIterator(const void* begin, u32 size)
	: m_cursor(static_cast<const char*>(begin))
	, m_current()
{
	urlDecode(const_cast<void*>(begin), size);
	m_end = m_cursor + strlen(m_cursor);
}
QueryStringIterator::~QueryStringIterator()
{
	m_cursor = nullptr;
	m_end = nullptr;
	m_current.key = nullptr;
	m_current.value = nullptr;
}

bool QueryStringIterator::more() const
{
	return m_cursor < m_end;
}
const QueryParameter& QueryStringIterator::moveNext()
{
	if(m_current.key)
	{
		memset(const_cast<char*>(m_current.key), 0, strlen(m_current.key));
		HeapFreeGlobal(const_cast<char*>(m_current.key));
	}
	if(m_current.value)
	{
		memset(const_cast<char*>(m_current.value), 0, strlen(m_current.value));
		HeapFreeGlobal(const_cast<char*>(m_current.value));
	}
	
	int keylen = indexOf('=', m_cursor);
	if(keylen == -1)
	{
		memset(&m_current, 0, sizeof(QueryParameter));
		m_cursor = m_end;
		return m_current;
	}

	char* key = (char*) HeapAllocGlobal(keylen + 1);
	memcpy(key, m_cursor, keylen);
	key[keylen] = '\0';
	m_current.key = key;
	m_cursor += keylen+1;

	int valuelen = indexOf('&', m_cursor);
	if(valuelen == -1)
	{
		valuelen = indexOf('\0', m_cursor);
		if(valuelen == -1)
		{
			m_cursor = m_end;
			return m_current;
		}
	}
		
	char* value = (char*) HeapAllocGlobal(valuelen + 1);
	memcpy(value, m_cursor, valuelen);
	value[valuelen] = '\0';
	m_current.value = value;
	m_cursor += valuelen+1;
	return m_current;
}

HttpSession::HttpSession(const char* hostname, u16 port)
	: m_sessionHandle(WinHttpOpen(L"app0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0))
	, m_hostname(hostname)
	, m_port(port)
	, m_https(port == INTERNET_DEFAULT_HTTPS_PORT)
{
	EASSERTF(m_sessionHandle, "Couldn't create http session (error: %d)", GetLastError());

	u32 len = strlen(hostname);
	wchar_t* hostnameWide = (wchar_t*)_alloca((len + 1) * sizeof(wchar_t));
	for(u32 i = 0; i < len; i++)
		hostnameWide[i] = wchar_t(hostname[i]);

	m_serverHandle = WinHttpConnect(m_sessionHandle, hostnameWide, INTERNET_PORT(port), 0);
	EASSERTF(m_serverHandle, "Couldn't connect to '%s' on port %u", hostname, port);
}

HttpSession::~HttpSession()
{
	if(m_serverHandle)
		WinHttpCloseHandle(m_serverHandle);

	if(m_sessionHandle)
		WinHttpCloseHandle(m_sessionHandle);
}

HINTERNET HttpSession::sessionHandle() const
{
	return m_sessionHandle;
}

HINTERNET HttpSession::handle() const
{
	return m_serverHandle;
}

const char* HttpSession::getHostname() const
{
	return m_hostname;
}

u16 HttpSession::getPort() const
{
	return m_port;
}

bool HttpSession::isHttps() const
{
	return m_https;
}

HttpClientRequestBuffer* HttpSession::tryRequest(HttpVerb method, const char* path, MIME accept, MIME content, Header* headers, u32 count, const void* payload, u32 len)
{
	const char* verb = verbName(method);
	size_t verblen = strlen(verb);
	wchar_t* verbw = (wchar_t*) _malloca((verblen + 1) * sizeof(wchar_t));
	for(size_t i = 0; i < verblen; i++)
		verbw[i] = wchar_t(verb[i]);
	verbw[verblen] = L'\0';

	const char* name = path;
	size_t namelen = strlen(name);
	wchar_t* namew = (wchar_t*) _malloca((namelen + 1) * sizeof(wchar_t));
	for(size_t i = 0; i < namelen; i++)
		namew[i] = wchar_t(name[i]);
	namew[namelen] = L'\0';

	LPCWSTR* accepts = WINHTTP_DEFAULT_ACCEPT_TYPES;

	LPCWSTR ourAccepts[2];
	if(accept != Mime_Any)
	{
		ourAccepts[0] = CMime::toWideString(accept);
		ourAccepts[1] = 0;
		accepts = ourAccepts;
	}

	HINTERNET hRequest = WinHttpOpenRequest(m_serverHandle, verbw, namew, NULL, NULL, accepts, m_https ? WINHTTP_FLAG_SECURE : 0);
	if(!hRequest)
	{
		return 0;
	}

	if(content != Mime_Any)
	{
		WideStringBuilder<> contentHeader;
		contentHeader << L"Content-Type: " << CMime::toWideString(content);
		BOOL bRes = WinHttpAddRequestHeaders(hRequest, contentHeader.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);
	}

	if(headers)
	{
		for(u32 i = 0; i < count; i++)
		{
			WideStringBuilder<> header;

			u32 headerNameLen = strlen(headers[i].name);
			wchar_t* headerNameWide = (wchar_t*)_malloca((headerNameLen + 1) * sizeof(wchar_t));
			for(u32 j = 0; j < headerNameLen; j++)
				headerNameWide[j] = wchar_t(headers[i].name[j]);
			headerNameWide[headerNameLen] = L'\0';			
			header.appendChars(headerNameWide, headerNameLen);
						
			header << L": ";

			u32 headerDataLen = strlen(headers[i].data);
			wchar_t* headerDataWide = (wchar_t*)_malloca((headerDataLen + 1) * sizeof(wchar_t));
			for(u32 j = 0; j < headerDataLen; j++)
				headerDataWide[j] = wchar_t(headers[i].data[j]);
			headerDataWide[headerDataLen] = L'\0';			
			header.appendChars(headerDataWide, headerDataLen);

			BOOL bRes = WinHttpAddRequestHeaders(hRequest, header.c_str(), DWORD(-1), WINHTTP_ADDREQ_FLAG_ADD);
			if(!bRes)
				EERRORF("Failed to add %s header", headers[i].name);
		}
	}

	if(!WinHttpSendRequest(hRequest, NULL, 0, (LPVOID)payload, (DWORD)len, (DWORD)len, NULL))
	{
		DWORD err = GetLastError();
		return 0;
	}

	if(!WinHttpReceiveResponse(hRequest, NULL))
	{
		return 0;
	}

	return new HttpClientRequestBuffer(hRequest);
}

HttpClientRequestBuffer::HttpClientRequestBuffer(void* handle)
	: Buffer()
	, m_requestHandle(handle)
	, m_contentLength(~0U)
{
	m_access = BIOACCESS(E_BIO_READ|E_BIO_FLUSH);
}

void* HttpClientRequestBuffer::handle() const
{
	return m_requestHandle;
}

u8* HttpClientRequestBuffer::data() const
{
	return 0;
}

u32 HttpClientRequestBuffer::size() const
{
	if(m_contentLength != ~0u)
		return m_contentLength;

	const_cast<HttpClientRequestBuffer*>(this)->m_contentLength = httpUtil::getNumber(m_requestHandle, WINHTTP_QUERY_CONTENT_LENGTH);
	return m_contentLength;
}
u64 HttpClientRequestBuffer::size64() const
{
	return size();	
}

void HttpClientRequestBuffer::flush()
{
	if(m_requestHandle)
	{
		WinHttpCloseHandle(m_requestHandle);
		m_requestHandle = nullptr;
	}
}

BIORETURN HttpClientRequestBuffer::read(void* Buffer, size_t Size)
{
	if(!m_requestHandle)
		return E_BIO_ERR_NO_IN_AVAIL;
	
	DWORD bytesRead = 0;

	if(!WinHttpReadData(m_requestHandle, Buffer, (DWORD)Size, &bytesRead))
	{
		return E_BIO_ERR_OS;
	}

	if(!bytesRead)
		return E_BIO_ERR_END_REACHED;

	return BIORETURN(bytesRead);
}

MIME HttpClientRequestBuffer::getContentType() const
{
	if(!m_requestHandle)
		return Mime_Unknown;

	wchar_t header[256]{0};
	DWORD headerlen = sizeof(header);

	if(!WinHttpQueryHeaders(m_requestHandle, WINHTTP_QUERY_CONTENT_TYPE, WINHTTP_HEADER_NAME_BY_INDEX, header, &headerlen, WINHTTP_NO_HEADER_INDEX))
		return Mime_Unknown;

	MIME m = CMime::fromWideString(header);
	return m;
}

SharedBuffer* HttpClientRequestBuffer::readShared()
{
	constexpr u32 kScatterSize = 2 MiB;

	if(!m_requestHandle)
		return 0;	

	BIORETURN err;
	AllocatingBuffer data;

	u8* scatter = (u8*) HeapAllocGlobal(kScatterSize);

	while(true)
	{
		err = read(scatter, kScatterSize);
		if(err == E_BIO_ERR_END_REACHED)
			break;

		size_t len = err;
		data.write(scatter, len);
	}

	return new MemorySharedBuffer(data.detach(), data.length());
}

void HttpClientRequest::setAcceptType(MIME type)
{
	m_acceptType = type;
}
void HttpClientRequest::setContentType(MIME type)
{
	m_contentType = type;
}

HttpClientRequest::HttpClientRequest(HttpSession& session, MIME acceptType, MIME contentType)
	: m_client(session)
	, m_acceptType(acceptType)
	, m_contentType(contentType)
{}

HttpClientRequestBuffer* HttpClientRequest::request(const char* uri, HttpVerb method, HttpSession::Header* headers, u32 count, const void* payload, u32 size) const
{
	ScopedRef buffer(m_client.tryRequest(method, uri, m_acceptType, m_contentType, headers, count, payload, size));
	if(buffer.isNull())
		return nullptr;

	if(m_acceptType != Mime_Any && m_acceptType == buffer->getContentType())
		return nullptr;

	return buffer.detach();
}
SharedBuffer* HttpClientRequest::requestShared(const char* uri, HttpVerb method, HttpSession::Header* headers, u32 count, const void* payload, u32 size) const
{
	ScopedRef buffer(m_client.tryRequest(method, uri, m_acceptType, m_contentType, headers, count, payload, size));
	if(buffer.isNull())
		return nullptr;

	if(m_acceptType != Mime_Any && m_acceptType != buffer->getContentType())
		return nullptr;

	return buffer->readShared();
}

SharedBuffer* HttpClientRequest::get(const char* uri, HttpSession::Header* headers, u32 count) const
{
	return requestShared(uri, HTTP_VERB_GET, headers, count);
}

HttpClientRequestBuffer* HttpClientRequest::rpcPRIVATE(const char* uri, const void* msg, HttpVerb method, u32 msgOptLen, MIME msgFormat, HttpSession::Header* headers, u32 count)
{
	setAcceptType(Mime_Any);
	setContentType(msgFormat);

	const void* payload = msg;
	u32 payloadSize = msgOptLen;

	switch(msgFormat)
	{
		case Mime_Json:
		{
			JsonConversionParams convParams;
			convParams.indentRoots = false;
			convParams.indentChildren = false;

			DbObject* dbo = (DbObject*) msg;
			StringBuilder<> jsonStr;
			dbo->toJsonString(jsonStr, convParams);

			payloadSize = jsonStr.size();
			payload = jsonStr.detach();
			break;
		}

		case Mime_Url:
		{
			eastl::vector<QueryParameter> params;
			for(DbObjectElementIterator it(*(DbObject*) msg); it.more();)
			{
				DbObjectElement e = it.next();
				
				StringBuilder<> value;
				e.getValueSafe(value);
				params.push_back({e.name(), value.detach()});
			}

			StringBuilder<> query;
			urlEncode(params.begin(), params.size(), query);

			payloadSize = query.size();
			payload = query.detach();
			break;
		}
	}

	return request(uri, method, headers, count, payload, payloadSize);
	//EASSERT(false, "Failed to convert response to a DbObject. Maybe the accept type is invalid for the the URI?");
}

DbObject HttpClientRequest::rpcOBJECT(const char* uri, const DbObject& msg, HttpVerb method, MIME msgFormat, HttpSession::Header* headers, u32 count)
{
	ScopedRef requestBuffer = rpcPRIVATE(uri, &msg, method, 0, msgFormat, headers, count);
	if(requestBuffer.isNull())
	{
		return c_NullObject;
	}

	switch(requestBuffer->getContentType())
	{
		case Mime_Json:
		{
			ScopedRef buffer = requestBuffer->readShared();

			DbObject obj;
			obj.fromJsonString((const char*) buffer->bytes(), buffer->size());
			return std::move(obj);
		}
		case Mime_Url:
		{
			ScopedRef buffer = requestBuffer->readShared();
			EASSERT(!buffer.isNull(), "Empty response, maybe the accept type is incorrect for the URI?");

			DbObjectBuilder builder;

			for(QueryStringIterator it(buffer->bytes(), buffer->size()); it.more();)
			{
				const QueryParameter& param = it.moveNext();
				builder << param.key << param.value;
			}

			return builder.done();
		}
		case Mime_Fson:
		{
			SharedBuffer* buffer = requestBuffer->readShared();
			return DbObject { static_cast<u8*>(buffer->bytes()), DatabaseOwnershipPolicy::Assume };
		}
	}

	return c_NullObject;
}

SharedBuffer* HttpClientRequest::rpcPAYLOAD(const char* uri, const SharedBuffer* payload, HttpVerb method, HttpSession::Header* headers, u32 count)
{
	ScopedRef requestBuffer = rpcPRIVATE(uri, payload ? payload->bytes() : nullptr, method, payload ? payload->size() : 0, Mime_OctetStream, headers, count);
	if(requestBuffer.isNull())
	{
		return nullptr;
	}

	return requestBuffer->readShared();
}

DbObject HttpClientRequest::getObject(const char* uri, HttpSession::Header* headers, u32 count)
{
	setAcceptType(Mime_Any);

	DbObject ret = c_NullObject;
	ScopedRef requestBuffer = request(uri, HTTP_VERB_GET, headers, count);

	switch(requestBuffer->getContentType())
	{
		case Mime_Json:
		{
			ScopedRef buffer = requestBuffer->readShared();

			DbObject obj;
			obj.fromJsonString((const char*) buffer->bytes(), buffer->size());
			return std::move(obj);
		}
		case Mime_Url:
		{
			ScopedRef buffer = requestBuffer->readShared();
			EASSERT(!buffer.isNull(), "Empty response, maybe the accept type is incorrect for the URI?");

			DbObjectBuilder builder;

			for(QueryStringIterator it(buffer->bytes(), buffer->size()); it.more();)
			{
				const QueryParameter& param = it.moveNext();
				builder << param.key << param.value;
			}

			return builder.done();
		}
		case Mime_Fson:
		{
			SharedBuffer* buffer = requestBuffer->readShared();
			return DbObject { static_cast<u8*>(buffer->bytes()), DatabaseOwnershipPolicy::Assume };
		}
	}

	EASSERT(false, "Failed to convert response to a DbObject. Maybe the accept type is invalid for the the URI?");
}

HttpClientRequestBuffer* HttpClientRequest::post(const char* uri, const void* payload, u32 size, HttpSession::Header* headers, u32 count) const
{
	return request(uri, HTTP_VERB_POST, headers, count, payload, size);
}

HttpClientRequestBuffer* HttpClientRequest::postJson(const char* uri, const char* data, HttpSession::Header* headers, u32 count)
{
	setAcceptType(Mime_Json);
	setContentType(Mime_Json);
	return request(uri, HTTP_VERB_POST, headers, count, data, (u32) strlen(data));
}

HttpClientRequestBuffer* HttpClientRequest::postObject(const char* uri, const DbObject& dbo, HttpSession::Header* headers, u32 count)
{
	setAcceptType(Mime_Fson);
	setContentType(Mime_Fson);
	return request(uri, HTTP_VERB_POST, headers, count, dbo.data(), dbo.dataSize());
}

HttpClientRequestBuffer* HttpClientRequest::postJson(const char* uri, const DbObject& data, HttpSession::Header* headers, u32 count)
{
	setAcceptType(Mime_Json);
	setContentType(Mime_Json);

	JsonConversionParams convParams;
	convParams.indentRoots = false;
	convParams.indentChildren = false;

	StringBuilder<> jsonStr;
	data.toJsonString(jsonStr, convParams);

	return request(uri, HTTP_VERB_POST, headers, count, jsonStr.c_str(), jsonStr.size());
}

HttpClientRequestBuffer* HttpClientRequest::postForm(const char* uri, const char* data, HttpSession::Header* headers, u32 count)
{	
	setAcceptType(Mime_Url);
	setContentType(Mime_Url);
	return request(uri, HTTP_VERB_POST, headers, count, data, (u32) strlen(data));
}

HttpClientRequestBuffer* HttpClientRequest::postForm(const char* uri, QueryParameter* params, u32 paramcount, HttpSession::Header* headers, u32 count)
{
	setAcceptType(Mime_Url);
	setContentType(Mime_Url);

	StringBuilder<> query;
	urlEncode(params, paramcount, query);
	return request(uri, HTTP_VERB_POST, headers, count, query.c_str(), query.size());
}
}