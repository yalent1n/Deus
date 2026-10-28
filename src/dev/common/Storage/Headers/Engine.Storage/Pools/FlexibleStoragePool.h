#pragma once
#include "../StoragePool.h"
#include <Engine.Base/Crypto/Guid.h>
#include <Engine.Base/Log.h>
#include <Engine.Base/Vec.h>
#include <Engine.Base/HashMap.h>
#include <Engine.Base/Thread/CriticalSection.h>
#include <Engine.Base/Buffer/Buffer.h>
#include <Engine.Core/VirtualFileSystem.h>

namespace rw
{
// Manages a directory using a catalog file and clusters.
// Every cluster is an independent storage-unit, which's
// files can be erased or kept when trimming.
// Trimming modes: partial trim, full trim.
// Partial trim only cuts down a given cluster's size
// without purging the link in the catalog file.
// With full trim, you can expect the erased files' links
// to be erased from the catalog file.
class CasStoragePool : public StoragePool, public IPartTrim, public IFullTrim
{
public:    
    // Note that the pool path can be a virtual path.
    // Like: "/mount/path/to/dir"
    FB_STORAGE_API CasStoragePool(VirtualFileSystem* vfs, const char* path, const char* name);
    FB_STORAGE_API virtual ~CasStoragePool();

public:
    struct Cluster;
    struct PendingWrite
    {
        const Cluster* cluster;
        u32 fileIndex;
        const SharedBuffer* data;
    };

    typedef Vec<PendingWrite> PendingWrites_t;

    struct Cluster
    {
        typedef Dict<ContentId, const SharedBuffer*> ChunkMap_t;

        struct ErasedFile
        {
            ChunkMap_t::iterator cache;
            u32 fileIndex;
            PendingWrites_t::iterator pending;
        };

        typedef Vec<ErasedFile> ErasedList_t;

        u32 id;
        u32 usedBytes;
        Buffer* stream;
        ChunkMap_t chunks;
        ErasedList_t erasedFiles;

        // utility functions
        inline Buffer* openStream(CasStoragePool& pool)
        {
            closeStream();

            FormattedStringBuilder<_MAX_PATH> path("%s%02x", pool.m_clusterDir, id);
            stream = pool.m_vfs->createFile(path.c_str(), true);
            EASSERTF(stream, "Failed to create cluster %u", id);
            usedBytes = stream->getAvailableBytes();
            return stream;
        }

        inline void closeStream()
        {
            if(stream)
            {
                delete stream;
                stream = nullptr;
            }
        }
    };

    typedef Dict<u32, Cluster> Clusters_t;

    struct CatalogInfo
    {
        typedef Vec<CatEntry> Files_t;
        typedef Vec<CatCryptoEntry> EncryptedFiles_t;
        typedef Vec<CatPatchEntry> PatchFiles_t;

        CasCatalogHeader header;
        Buffer* stream;
        Files_t files;
        PatchFiles_t patchFiles;
        EncryptedFiles_t encryptedFiles;
    };     

public:
    FB_STORAGE_API virtual void                        updateConfig(const StoragePoolConfig& config) override;
    FB_STORAGE_API virtual void                        flush(bool trim = true) override;
    FB_STORAGE_API virtual DbObject                    describe() override;

private:
    FB_STORAGE_API InsertResult appendChunk(const Cluster* extent, const SharedBuffer* data, CatEntry* file);
    FB_STORAGE_API FetchResult readChunk(const CatEntry& file, SharedBuffer** out);
    FB_STORAGE_API FetchResult readChunkPatched(const CatEntry& base, const CatEntry& delta, SharedBuffer** out);
    FB_STORAGE_API bool readChunk(const CatEntry& file, void* buf, u32* size);

public:
    FB_STORAGE_API virtual InsertResult                putPatch(ContentId* digest, const ContentId& base, const SharedBuffer* delta) override;
    FB_STORAGE_API virtual InsertResult                putPatch(ContentId* digest, const ContentId& base, const ContentId& delta) override;
    FB_STORAGE_API virtual FetchResult                 getPatch(const ContentId& digest, ContentId& base, ContentId& delta) override;
    
    // Put raw chunk into the pending writes list.
    FB_STORAGE_API virtual InsertResult                putChunk(const SharedBuffer* chunk) override;
    FB_STORAGE_API virtual FetchResult                 getChunk(const ContentId& digest, SharedBuffer** out) override;
    FB_STORAGE_API virtual bool                        hasChunk(const ContentId& digest) override;
    FB_STORAGE_API virtual FetchResult                 fetchChunk(const ContentId& digest, void* buf, u32* size) override {}
    FB_STORAGE_API virtual bool                        dropChunks(const ContentId* digests, u32 count) override;
    FB_STORAGE_API void                                iterateChunks(ChunkCallback_t&& cb) override;

    FB_STORAGE_API bool collectGarbage();


    FB_STORAGE_API virtual IPartTrim* getPartTrim() override { return this; }
    FB_STORAGE_API virtual IFullTrim* getFullTrim() override { return this; }

    // Trims a cluster in the clusters directory to the smallest size it can do, and only does this.
    FB_STORAGE_API virtual bool partTrim(u32 id) override;

    // Full trim does an "snapshot" of the curently in use files and, exports them into the actual directory.
    FB_STORAGE_API virtual bool fullTrim() override;

// Extent functions
private:
    Cluster&     openCluster(u32 cluster);
    Cluster&     createCluster(u32 cluster);
    void         makeWritableCluster(u32 size);
    void         trimClusterSpace(Cluster& cluster);

// Catalog functions
private:
    void readCatalog();
    void writeCatalog();

private:
    VirtualFileSystem* m_vfs;
    const char* m_dir;
    const char* m_clusterDir;

    u32 m_clusterId;
    Cluster* m_cluster;
    Clusters_t m_clusters;

    CatalogInfo m_catalog;    

    CriticalSection m_cs;
    PendingWrites_t m_pendingWrites;
};
}