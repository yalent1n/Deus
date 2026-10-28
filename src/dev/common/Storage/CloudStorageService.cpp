#include <Engine.Storage/CloudStorageService.h>
#include <Engine.Base/Buffer/Adapter.h>

namespace rw
{
struct CsCommHeader
{
    static inline constexpr u32 kMagic = 0xC4554DE;

    u32 magic;
    CloudStorageMessageType comm;
    u32 length;
    u8 payload[1];
};

struct CloudStorageMessage
{
    AllocatingBuffer messageBuffer;
    CloudStorageServerConnection* connection;

    /* return true if more read is available */
    bool nextRead();
};

bool CloudStorageMessage::nextRead()
{
    bool done = false;
    while(ScopedRef buf = connection->dequeueRead())
    {
        messageBuffer.write(buf->bytes(), buf->size());
        if(buf->size() < connection->getStorageServer()->getReadBlockSize())
        {
            done = true;
            break;
        }
    }

    return !done;
}

CloudStorageServerConnection::CloudStorageServerConnection(CloudStorageServer* storageServer)
    : m_storageServer(storageServer)
{}

CloudStorageServerConnection::~CloudStorageServerConnection()
{

}

void CloudStorageServerConnection::onDataAvailable()
{
    if(!m_message)
        m_message = new CloudStorageMessage();
    
    if(m_message->nextRead())
        return;

    m_storageServer->onMessage(*m_message);
}

void CloudStorageServerConnection::onConnectionEnd()
{

}

void CloudStorageServerConnection::onDisconnect()
{

}

CloudStorageServer::CloudStorageServer(CloudStorageService* storageService, WorkerThreadPool* workerThreadPool, u16 port)
    : TcpServer(port, workerThreadPool, true)
    , m_storageService(storageService)
{}

CloudStorageServer::~CloudStorageServer()
{

}

TcpServerConnection* CloudStorageServer::newConnection()
{
    return new CloudStorageServerConnection(this);
}

void CloudStorageServer::onConnect(TcpServerConnection* connection)
{

}

void CloudStorageServer::onDisconnect(TcpServerConnection* connection)
{

}

void CloudStorageServer::onMessage(const CloudStorageMessage& message)
{
    CsCommHeader* hdr = (CsCommHeader*)message.messageBuffer.detach();
    if(hdr->magic != CsCommHeader::kMagic)
    {
        EERROR("(warning) a Cloud Storage message is invalid");
        return;
    }
    
    m_storageService->onMessage(hdr->comm, hdr->payload, hdr->length);
}

CloudStorageService::CloudStorageService(VirtualFileSystem* vfs, WorkerThreadPool* workerThreadPool, StorageFormat format, const char* poolPath)
    : m_localStoragePool(nullptr)
{
    switch(format)
    {
        case StorageFormat::File:
            m_localStoragePool = new FileStoragePool(vfs, poolPath, getFriendlyName());
            break;
        case StorageFormat::Flexible:
            m_localStoragePool = new CasStoragePool(vfs, poolPath, getFriendlyName());
            break;
        default:
            EASSERT(false, "Unhandled storage format for Cloud Storage Service");
            break;
    }

    m_storageServer = new CloudStorageServer(this, workerThreadPool, 1890);
}

CloudStorageService::~CloudStorageService()
{
    if(m_storageServer)
        m_storageServer->shutdown();

    delete m_storageServer;
    m_storageServer = nullptr;

    if(m_localStoragePool)
        m_localStoragePool->flush();

    delete m_localStoragePool;
    m_localStoragePool = nullptr;
}

void CloudStorageService::onMessage(CloudStorageMessageType type, const u8* payload, u32 length)
{
    switch(type)
    {
        case CSC_ANNOUNCE: onAnnounce(payload, length); break;
        case CSC_PUSH: onPush(payload, length); break;
        case CSC_PUSH_PATCH: onPushPatch(payload, length); break;
        case CSC_PULL: onPull(payload, length); break;        
        case CSC_PULL_PATCH: onPullPatch(payload, length); break;
        case CSC_QUERY_INFO: onQueryInfo(payload, length); break;        
        case CSC_QUERY_PATCH_INFO: onQueryPatchInfo(payload, length); break;

        default:
        case CSC_UNDEF:
            break;
    }
}

void CloudStorageService::onAnnounce(const u8* payload, u32 length)
{
    Adapter data(payload, length);
    const char* announcer = data.decodeCString();
    if(isnil(announcer))
    {
        // TODO: handle error
        return;
    }

    u32 hash = hashQuickLowerCase(announcer);

    /*
        Namespaces:
        - Global namespaces:
        /public/_s3/{bucket} <- global/shared object storage separated by buckets

        - User namespaces:
        /private/deus/_kv/global <- global references
        /private/deus/_kv/metadata <- metadata objects like uid, bucket keys, hosts (no host separation), etc.
        /private/deus/_kv/keys <- key checksum table for bucket keys
        /private/deus/_kv/mapping <- file name <-> content hash
        /private/deus/_s3/{bucket} <- object storage separated by buckets
    */

    StringBuilder<> userPath;
    userPath << "users/" << hash << "/s3/";
}

void CloudStorageService::onPush(const u8* payload, u32 length)
{
    InsertResult result = m_localStoragePool->putChunk(payload, length);
    if(result != InsertResult::Inserted && result != InsertResult::AlreadyPresent)
    {
        // TODO: handle error
        return;
    }

    // TODO: handle response (send result)
}

void CloudStorageService::onPushPatch(const u8* payload, u32 length)
{
    InsertResult result = m_localStoragePool->putChunk(payload, length, Mime_Patch);
    if(result != InsertResult::Inserted && result != InsertResult::AlreadyPresent)
    {
        // TODO: handle error
        return;
    }

    // TODO: handle response (send result & patch hash)
}

void CloudStorageService::onPull(const u8* payload, u32 length)
{
    if(length != sizeof(SHA1))
        return;

    const SHA1* hash = (const SHA1*)payload;
    const SharedBuffer* content = m_localStoragePool->getChunk(*hash);
    if(!content)
    {
        // TODO: handle error
        return;
    }

    // TODO: handle response (send content)
}

void CloudStorageService::onPullPatch(const u8* payload, u32 length)
{
    if(length != sizeof(SHA1))
        return;

    const SHA1* hash = (const SHA1*)payload;
    const SharedBuffer* content = m_localStoragePool->getChunk(*hash);
    if(!content)
    {
        // TODO: handle error
        return;
    }

    // TODO: handle response (send patch data)
}

void CloudStorageService::onQueryInfo(const u8* payload, u32 length)
{
    if(length != sizeof(SHA1))
        return;

    const SHA1* hash = (const SHA1*)payload;
    const SharedBuffer* content = m_localStoragePool->getChunk(*hash);
    if(!content)
    {
        // TODO: handle error
        return;
    }

    // TODO: handle response (send hash & size)
}

void CloudStorageService::onQueryPatchInfo(const u8* payload, u32 length)
{
    if(length != sizeof(SHA1))
        return;

    const SHA1* hash = (const SHA1*)payload;
    const SharedBuffer* content = m_localStoragePool->getChunk(*hash);
    if(!content)
    {
        // TODO: handle error
        return;
    }

    // TODO: handle response (send hash & size of patch data)
}
}