#pragma once
#include "../StoragePool.h"
#include <Engine.Network/HttpSession.h>

namespace rw
{
class RemoteStoragePool : public StoragePool, public IFullTrim
{
public:
    FB_STORAGE_API RemoteStoragePool(const char* host);
    FB_STORAGE_API ~RemoteStoragePool();

public:
    FB_STORAGE_API virtual void                        updateConfig(const StoragePoolConfig& config) override;
    FB_STORAGE_API virtual void                        flush(bool trim = false) override;
    FB_STORAGE_API virtual DbObject                    describe() override;

    FB_STORAGE_API virtual InsertResult                putPatch(ContentId* digest, const ContentId& base, const SharedBuffer* delta) override;
    FB_STORAGE_API virtual InsertResult                putPatch(ContentId* digest, const ContentId& base, const ContentId& head) override;
    FB_STORAGE_API virtual FetchResult                 getPatch(const ContentId& digest, ContentId& base, ContentId& delta) override;
    
    // Put raw chunk into the pending writes list.
    FB_STORAGE_API virtual InsertResult                putChunk(const SharedBuffer* chunk) override;
    FB_STORAGE_API virtual FetchResult                 getChunk(const ContentId& digest, SharedBuffer** out) override;
    FB_STORAGE_API virtual FetchResult                 fetchChunk(const ContentId& digest, void* buf, u32* size) override;
    FB_STORAGE_API virtual bool                        hasChunk(const ContentId& digest) override;
    FB_STORAGE_API virtual bool                        dropChunks(const ContentId* digests, u32 count) override;
    FB_STORAGE_API void                                iterateChunks(ChunkCallback_t&& cb) override;

    FB_STORAGE_API virtual IPartTrim* getPartTrim() override { return nullptr; }
    FB_STORAGE_API virtual IFullTrim* getFullTrim() override { return this; }

    // Full trim does an "snapshot" of the curently in use files and, exports them into the actual directory.
    FB_STORAGE_API virtual bool fullTrim() override;

private:
    HttpSession m_StorageSession;
};
}