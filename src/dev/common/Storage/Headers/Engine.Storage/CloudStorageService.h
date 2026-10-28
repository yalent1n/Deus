#pragma once
#include "Bundle.h"
#include "Pools/FileStoragePool.h"
#include "Pools/FlexibleStoragePool.h"
#include <Engine.ToolCore/App/ServiceBase.h>
#include <Engine.Network/TcpServer.h>

namespace rw
{
class CloudStorageMessage;
class CloudStorageServer;
class CloudStorageServerConnection;
class CloudStorageService;

enum CloudStorageMessageType
{
    CSC_UNDEF,
    CSC_ANNOUNCE,
    CSC_PUSH,
    CSC_PUSH_PATCH,
    CSC_PULL,
    CSC_PULL_PATCH,
    CSC_QUERY_INFO,
    CSC_QUERY_PATCH_INFO
};

class CloudStorageServerConnection : public TcpServerConnection
{
public:
    FB_NETWORK_API CloudStorageServerConnection(CloudStorageServer*);
    FB_NETWORK_API ~CloudStorageServerConnection();

    CloudStorageServer* getStorageServer() const
        { return m_storageServer; }

    FB_NETWORK_API void onDataAvailable() override;
	FB_NETWORK_API void onConnectionEnd() override;
	FB_NETWORK_API void onDisconnect() override;

private:
    CloudStorageServer* m_storageServer;
    CloudStorageMessage* m_message;
};

class CloudStorageServer : public TcpServer
{
public:
    FB_STORAGE_API CloudStorageServer(CloudStorageService* storageService, WorkerThreadPool* workerThreadPool, u16 port);
    FB_STORAGE_API ~CloudStorageServer();

    CloudStorageService* getStorageService() const
        { return m_storageService; }

	FB_STORAGE_API TcpServerConnection* newConnection() override;
	FB_STORAGE_API void onConnect(TcpServerConnection* connection) override;
	FB_STORAGE_API void onDisconnect(TcpServerConnection* connection) override;

    FB_STORAGE_API void onMessage(const CloudStorageMessage& message);

private:
    CloudStorageService* m_storageService;
};

class CloudStorageService : public ServiceBase
{
public:
    /* poolPath has to be ended with a separator */
    FB_STORAGE_API CloudStorageService(VirtualFileSystem* vfs, WorkerThreadPool* workerThreadPool, StorageFormat format, const char* poolPath);
    FB_STORAGE_API ~CloudStorageService();

    const char* getFriendlyName() const override
        { return "CloudStorageService"; }

    CloudStorageServer* getStorageServer() const
        { return m_storageServer; }

    StoragePool* getLocalStoragePool() const
        { return m_localStoragePool; }

    FB_STORAGE_API void onMessage(CloudStorageMessageType type, const u8* payload, u32 length);

private:
    void onAnnounce(const u8* payload, u32 length);
    void onPush(const u8* payload, u32 length);
    void onPushPatch(const u8* payload, u32 length);
    void onPull(const u8* payload, u32 length);
    void onPullPatch(const u8* payload, u32 length);
    void onQueryInfo(const u8* payload, u32 length);
    void onQueryPatchInfo(const u8* payload, u32 length);

private:
    CloudStorageServer* m_storageServer;
    StoragePool* m_localStoragePool;
};
}