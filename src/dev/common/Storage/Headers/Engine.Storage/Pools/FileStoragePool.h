#pragma once
#include "../StoragePool.h"
#include <Engine.Base/Crypto/SHA.h>

namespace rw
{
class VirtualFileSystem;
class StringEditor;

// Manages a directory's content using the content key.
// Note:
// - A chunk's digest is broken up into shards:
//      2 bytes, 2 bytes, 16 bytes.
// - This way chunks are grouped by content-similarity.
class FileStoragePool : public StoragePool
{
public:
    // Note that the pool path can be a virtual path.
    // Like: "/mount/path/to/dir"
    FB_STORAGE_API          FileStoragePool(VirtualFileSystem*, const char* poolPath, const char* poolName = "");
    FB_STORAGE_API virtual ~FileStoragePool();

    FB_STORAGE_API virtual void                        updateConfig(const StoragePoolConfig& config) override;
    FB_STORAGE_API virtual void                        flush(bool trim = true) override;
    FB_STORAGE_API virtual DbObject                    describe() override;
    
    FB_STORAGE_API virtual InsertResult                putPatch(ContentId* digest, const ContentId& base, const SharedBuffer* chunk) override;
    FB_STORAGE_API virtual InsertResult                putPatch(ContentId* digest, const ContentId& base, const ContentId& delta) override;
    FB_STORAGE_API virtual FetchResult                 getPatch(const ContentId& digest, ContentId& base, ContentId& delta) override;
    FB_STORAGE_API virtual InsertResult                putChunk(const SharedBuffer* content) override;
    FB_STORAGE_API virtual FetchResult                 getChunk(const ContentId& digest, SharedBuffer** out) override;
    FB_STORAGE_API virtual FetchResult                 fetchChunk(const ContentId& digest, void* buf, u32* size) override;
    FB_STORAGE_API virtual bool                        hasChunk(const ContentId& digest) override;
    FB_STORAGE_API virtual bool                        dropChunks(const ContentId* digests, u32 count) override;
    FB_STORAGE_API void                                iterateChunks(ChunkCallback_t&& cb) override;

    FB_STORAGE_API virtual IPartTrim* getPartTrim() override { return nullptr; }
    FB_STORAGE_API virtual IFullTrim* getFullTrim() override { return nullptr; }

private:
    void makeKeyPath(const ContentId& digest, StringEditor& result) const;

private:
    VirtualFileSystem* m_ioManager;
    const char* m_rootPath;
};
}