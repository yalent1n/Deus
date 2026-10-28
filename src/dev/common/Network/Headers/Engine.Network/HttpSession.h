#pragma once
#include "Bundle.h"
#include "HttpBase.h"

#include <Engine.Base/Buffer/Buffer.h>
#include <Engine.Base/Buffer/SharedBuffer.h>
#include <Engine.Base/Database/DbObject.h>

namespace rw
{
class HttpClientRequestBuffer;

// Helper class to interact with the host.
class HttpSession
{
public:
	FB_NETWORK_API HttpSession(const char* hostname, u16 port = 443); // default to HTTPS
	FB_NETWORK_API ~HttpSession();

	struct Header
	{
		const char* name;
		const char* data;
	};

	// Returns the handle from WinHttpOpen.
	FB_NETWORK_API void* sessionHandle() const;

	// Returns the handle from WinHttpConnect, this is used for later purposes.
	FB_NETWORK_API void* handle() const;

	FB_NETWORK_API const char* getHostname() const;
	FB_NETWORK_API u16 getPort() const;
	FB_NETWORK_API bool isHttps() const;

	// Tries to send a request to the specified path.
	// @param method: Request method
	// @param path: The thing after the hostname
	// @param headers: OPTIONAL - additional headers
	// @param count: Additional headers count
	// @param payload: OPTIONAL - content that will be sent after the headers
	// @param len: Length of the payload
	FB_NETWORK_API HttpClientRequestBuffer* tryRequest(HttpVerb method, const char* path, MIME accept = Mime_Any, MIME content = Mime_Any, Header* headers = 0, u32 count = 0, const void* payload = 0, u32 len = 0);

private:
	void* m_sessionHandle;
	void* m_serverHandle;

	const char* m_hostname;
	u16 m_port;
	bool m_https;
};

class HttpClientRequestBuffer : public Buffer
{
public:
	FB_NETWORK_API HttpClientRequestBuffer(void* hReq);

	FB_NETWORK_API void* handle() const;
	FB_NETWORK_API u8* data() const;

	// Overrides	
	FB_NETWORK_API virtual u32				size() const;
	FB_NETWORK_API virtual u64				size64() const;
	FB_NETWORK_API virtual void				flush() override;
	FB_NETWORK_API virtual void				seek(size_t, BIOSEEKDIR) override { EASSERT(false, "not seekable"); }
	FB_NETWORK_API virtual BIORETURN		read(void* Buffer, size_t Size) override;

	////////////

	FB_NETWORK_API MIME						getContentType() const;
	FB_NETWORK_API SharedBuffer*			readShared();

private:
	void* m_requestHandle;
	u32 m_contentLength;
};

class HttpClientRequest
{
public:
	FB_NETWORK_API HttpClientRequest(HttpSession& session, MIME acceptType = Mime_Any, MIME contentType = Mime_Any);

	FB_NETWORK_API void setAcceptType(MIME type);
	FB_NETWORK_API void setContentType(MIME type);

	FB_NETWORK_API HttpClientRequestBuffer*			request(const char* uri, HttpVerb method, HttpSession::Header* headers = 0, u32 count = 0, const void* payload = 0, u32 size = 0) const;
	FB_NETWORK_API SharedBuffer*					requestShared(const char* uri, HttpVerb method, HttpSession::Header* headers = 0, u32 count = 0, const void* payload = 0, u32 size = 0) const;

	// Sends a GET request and returns the response content.
	// @param uri: Path to the resource
	// @param headers: OPTIONAL - additional headers
	// @param count: Header count
	FB_NETWORK_API SharedBuffer*					get(const char* uri, HttpSession::Header* headers = 0, u32 count = 0) const;

private:
	HttpClientRequestBuffer*						rpcPRIVATE(const char* uri, const void* msg = NULL, HttpVerb method = HTTP_VERB_POST, u32 msgOptLen = 0, MIME msgFormat = Mime_Json, HttpSession::Header* headers = nullptr, u32 count = 0);

public:
	// Sends a GET request that accepts any response that's convertible to a property table.
	// Acceptable types: Mime_Url, Mime_Json, Mime_Fson.
	// @param uri: Path to the resource
	// @param headers: OPTIONAL - additional headers
	// @param count: Header count
	FB_NETWORK_API DbObject							rpcOBJECT(const char* uri, const DbObject& dbo = c_NullObject, HttpVerb method = HTTP_VERB_POST, MIME msgFormat = Mime_Json, HttpSession::Header* headers = nullptr, u32 count = 0);
	FB_NETWORK_API SharedBuffer*					rpcPAYLOAD(const char* uri, const SharedBuffer* payload = NULL, HttpVerb method = HTTP_VERB_POST, HttpSession::Header* headers = nullptr, u32 count = 0);

	// Sends a GET request that accepts any response that's convertible to a property table.
	// Acceptable types: Mime_Url, Mime_Json, Mime_Fson.
	// @param uri: Path to the resource
	// @param headers: OPTIONAL - additional headers
	// @param count: Header count
	FB_NETWORK_API DbObject							getObject(const char* uri, HttpSession::Header* headers = nullptr, u32 count = 0);
	
	// Sends a POST request and returns the response buffer ready to be read.
	// @param uri: Upload path
	// @param payload: Data to be sent
	// @param size: Payload size
	// @param headers: OPTIONAL - additional headers
	// @param count: Header count
	FB_NETWORK_API HttpClientRequestBuffer*			post(const char* uri, const void* payload, u32 size, HttpSession::Header* headers = 0, u32 count = 0) const;

	// Sends a POST request with a JSON body and returns the response buffer ready to be read.
	// @param uri: Upload path
	// @param data: Raw json string
	FB_NETWORK_API HttpClientRequestBuffer*			postJson(const char* uri, const char* data, HttpSession::Header* headers = 0, u32 count = 0);

	// Sends a POST request with a binary JSON body and returns the response buffer ready to be read.
	// @param uri: Upload path
	// @param data: Raw json string
	FB_NETWORK_API HttpClientRequestBuffer*			postObject(const char* uri, const DbObject& fson, HttpSession::Header* headers = nullptr, u32 count = 0);
	
	// Sends a POST request with a JSON body and returns the response buffer ready to be read.
	// @param uri: Upload path
	// @param data: Object to be transformed into json
	FB_NETWORK_API HttpClientRequestBuffer*			postJson(const char* uri, const DbObject& data, HttpSession::Header* headers = 0, u32 count = 0);

	// Sends a POST request with a x-www-form-urlencoded body and returns the response buffer ready to be read.
	// @param uri: Upload path
	// @param data: Raw encoded string
	FB_NETWORK_API HttpClientRequestBuffer*			postForm(const char* uri, const char* data, HttpSession::Header* headers = 0, u32 count = 0);
	
	// Sends a POST request with a x-www-form-urlencoded body and returns the response buffer ready to be read.
	// @param uri: Upload path
	// @param params: Query parameter
	FB_NETWORK_API HttpClientRequestBuffer*			postForm(const char* uri, QueryParameter* params, u32 paramcount, HttpSession::Header* headers = 0, u32 count = 0);

private:
	HttpSession& m_client;
	MIME m_acceptType;
	MIME m_contentType;
};
}