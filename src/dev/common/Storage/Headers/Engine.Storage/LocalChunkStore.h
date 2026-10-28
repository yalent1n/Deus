#pragma once
#include "StoragePool.h"
#include "Deployment/BuildImage.h"

namespace rw
{
struct LocalChunkStoreEntry
{
    const BuildImageInstallChunk* ichunk;
    StoragePool* pool;
};

class LocalChunkStore
{
public:
    FB_STORAGE_API LocalChunkStore(eastl::string name, BuildLayerRoot& root);
    FB_STORAGE_API ~LocalChunkStore();

    // Returns the index of the pool entry.
    FB_STORAGE_API uint addPool(StoragePool& pool, BuildImageInstallChunk& icbl);
    
    // Returns the index of the pool entry.
    FB_STORAGE_API uint createFileStorage(BuildImageInstallChunk& icbl);
    
    // Returns the index of the pool entry.
    FB_STORAGE_API uint createFlexibleStorage(BuildImageInstallChunk& icbl);
    
    // Removes the pool entry from the chunk store.
    FB_STORAGE_API void removePool(uint index);

    // Searches which pool is assocciated with the store entry.
    FB_STORAGE_API StoragePool* getPool(uint index) const;
    
    // Searches which install chunk is assocciated with the store entry.
    FB_STORAGE_API const BuildImageInstallChunk* getInstallChunk(uint index) const;
    
    // Searches which root is assocciated with the store entry.
    FB_STORAGE_API const BuildLayerRoot* getLayer() const;

private:
    eastl::string m_name;
    BuildLayerRoot& m_root;
    Vec<LocalChunkStoreEntry> m_chunkStoreEntries;
};
}