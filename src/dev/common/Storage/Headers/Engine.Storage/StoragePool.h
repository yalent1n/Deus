#pragma once
#include "Bundle.h"
#include "CatalogTypes.h"
#include <functional>
#include <Engine.Base/Buffer/SharedBuffer.h>
#include <Engine.Base/Database/DbObjectBuilder.h>

namespace rw
{
enum class InsertResult : u8
{
    AlreadyPresent,
    Inserted,

    Error,
    NoSpace,
    NullFeedback,
    NoSourceSpecified,
    BaseNotFound,
    DeltaNotComputed
};

enum class FetchResult : s8
{
    NotFound = -6,
    BaseNotFound,
    DeltaNotFound,
    NotFoundCatalog,
    NoTargetSpecified,
    SysApiError,

    Found,
    FoundCold, // temporarily in memory, if the storage shuts down it goes with it
    FoundHot, // permamently on disk and temporarily in memory, till the storage shuts down
};

enum class StorageFormat : u8
{
    File,           // files are sharded across inside pool directory
    Remote,         // probably connects over HTTP using CasContentIdContentService
    Flexible,       // uses extent files
    Fixed           // uses cas files
};

FB_STORAGE_API StorageFormat toStorageFormat(const char* format);

struct StoragePoolConfig
{
    StorageFormat poolFormat = StorageFormat::Flexible;
    const char* poolName;
    const char* poolPath;
    bool quickFlush = true;
    u32 clusterSizeMB = 1024;
    u32 alignment = 1 KiB;
};

struct CasTrimContext
{
    const ContentId* digests;
    u32 count;
};

interface IPartTrim
{
    virtual bool partTrim(u32 id) = 0;
};

interface IFullTrim
{    
    virtual bool fullTrim() = 0;
};

class StoragePool
{
public:
    typedef std::function<void(const ContentId&)> ChunkCallback_t;

    FB_STORAGE_API          StoragePool();
    FB_STORAGE_API virtual ~StoragePool();

    const StoragePoolConfig& getConfiguration() const { return m_config; }
    bool isLocal() const { return m_config.poolFormat != StorageFormat::Remote; }

    virtual void                        updateConfig(const StoragePoolConfig& config) = 0;
    virtual void                        flush(bool trim = true) = 0;
    virtual DbObject                    describe() = 0;
    
    virtual InsertResult                putPatch(ContentId* digest, const ContentId& base, const SharedBuffer* chunk) = 0;
    virtual InsertResult                putPatch(ContentId* digest, const ContentId& base, const ContentId& delta) = 0;
    virtual FetchResult                 getPatch(const ContentId& digest, ContentId& base, ContentId& delta) = 0;
    virtual InsertResult                putChunk(const SharedBuffer* content) = 0;
    virtual FetchResult                 getChunk(const ContentId& digest, SharedBuffer** out) = 0;
    virtual FetchResult                 fetchChunk(const ContentId& digest, void* buf, u32* size) = 0;

    virtual bool                        hasChunk(const ContentId& digest) = 0;
    virtual bool                        dropChunks(const ContentId* digests, u32 count) = 0;
    virtual void                        iterateChunks(ChunkCallback_t&& cb) = 0;

    virtual IPartTrim* getPartTrim() = 0;
    virtual IFullTrim* getFullTrim() = 0;

    bool                                dropChunk(const ContentId& digest)
    {
        return dropChunks(&digest, 1);
    }
    
    InsertResult                        putChunk(const void* data, u32 size, MIME mime = Mime_Unknown)
    {
        auto sb = new MemorySharedBuffer(data, size);
        sb->setMimeType(mime);
        return putChunk(sb);
    }
protected:
    StoragePoolConfig m_config;
};
}