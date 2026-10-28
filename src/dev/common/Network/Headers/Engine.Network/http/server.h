#pragma once
#include "../TcpServer.h"
#include "../HttpBase.h"
#include "router.h"
#include <Engine.Base/Buffer/Adapter.h>

#define NET_HTTP_MTU 8096

namespace rw
{
class HttpServer;

class HttpServerConnection : public TcpServerConnection
{
public:
    AllocatingBuffer Buffer;

public:
    HttpServerConnection(HttpServer* srv);

    void onDataAvailable() override;
	void onConnectionEnd() override;
	void onDisconnect() override;

    struct SendAdditionalData
    {
        eastl::vector<HttpHeader> Headers;
        MIME ContentType;
        CodecMethod ContentEncoding;
        const char* Content;
        int ContentLength;
    };

    void EnqueueResponse(HttpStatus status, const SendAdditionalData* additionalData = NULL);
    
private:
    const char* FetchHeaderText(const char* Key) const;

    void ProcessPayload(const char* Payload, MIME ContentType, const char* ContentEncoding);
    size_t ProgressData(char* Cursor, size_t AmtRead);
    size_t ProgressInitial(char* Cursor, size_t AmtRead);

private:
    HttpRequest     m_Request;

    HttpServer*     m_Srv;
    int             m_BytesProgressed;
    int             m_ContentBytesProgessed;
    int             m_State;

    int             m_PathOffset;

    int             m_CurrentHeaderKeyOffset;
    int             m_CurrentHeaderValueOffset;

    int             m_ContentOffset;
    int             m_ContentLength;

};

class HttpServer : public TcpServer
{
public:
    FB_NETWORK_API HttpServer(u16 Port, WorkerThreadPool* ThreadPool, bool Https = true);
    FB_NETWORK_API ~HttpServer();

	FB_NETWORK_API TcpServerConnection* newConnection() override;
	FB_NETWORK_API void onConnect(TcpServerConnection* connection) override;
	FB_NETWORK_API void onDisconnect(TcpServerConnection* connection) override;

    FB_NETWORK_API void update();

    FB_NETWORK_API HttpRouter* addRouter(const char* path);
    FB_NETWORK_API void deleteRouter(HttpRouter* router);
    FB_NETWORK_API HttpRouter* fetchRouterFromUri(const char* uri);
    FB_NETWORK_API HttpRouter* fetchRouterAndCaptureFromUri(const char* uri, eastl::vector<http_path_param>& captures);
    
    FB_NETWORK_API void enqueue(HttpRequest& req);
    FB_NETWORK_API virtual void error(HttpStatus status, HttpRequest& req);
    FB_NETWORK_API void complete(HttpRequest& req);

private:
    eastl::vector<HttpRequest*> m_RequestQueue;
    eastl::vector<HttpRouter> m_Routers;
};
}