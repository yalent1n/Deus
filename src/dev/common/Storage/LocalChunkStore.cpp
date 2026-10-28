#include <Engine.Storage/LocalChunkStore.h>

namespace rw
{
LocalChunkStore::LocalChunkStore(eastl::string name, BuildLayerRoot& root)
    : m_name(name)
    , m_root(root)
{}

LocalChunkStore::~LocalChunkStore()
{
    for(const LocalChunkStoreEntry& entry : m_chunkStoreEntries)
    {
        StoragePool* icStorage = entry.pool;
        icStorage->flush();
        delete icStorage;
    }
}

uint LocalChunkStore::addPool(StoragePool& pool, BuildImageInstallChunk& ichunk)
{
    LocalChunkStoreEntry& entry = m_chunkStoreEntries.push_back();
    entry.ichunk = &ichunk;
    entry.pool = &pool;
    return uint(&entry - m_chunkStoreEntries.begin());
}

uint LocalChunkStore::createFileStorage(BuildImageInstallChunk& ichunk)
{
    StringBuilder<> installBundle;
    BuildLayerFileFullPath(m_root, ichunk.getInstallBundle()).getFilePath(installBundle);
    ELOGF("%s - Create file storage pool %s", m_name.c_str(), ichunk.getInstallBundle());
    return 0;
}

uint LocalChunkStore::createFlexibleStorage(BuildImageInstallChunk& ichunk)
{
    StringBuilder<> installBundle;
    BuildLayerFileFullPath(m_root, ichunk.getInstallBundle()).getFilePath(installBundle);
    ELOGF("%s - Create flexible storage pool %s", m_name.c_str(), ichunk.getInstallBundle());
    return 0;
}

void LocalChunkStore::removePool(uint index)
{
    if(index < m_chunkStoreEntries.size())
        m_chunkStoreEntries.erase(m_chunkStoreEntries.begin() + index);
}

StoragePool* LocalChunkStore::getPool(uint index) const
{
    if(index < m_chunkStoreEntries.size())
        return m_chunkStoreEntries[index].pool;

    return nullptr;
}

const BuildImageInstallChunk* LocalChunkStore::getInstallChunk(uint index) const
{
    if(index < m_chunkStoreEntries.size())
        return m_chunkStoreEntries[index].ichunk;

    return nullptr;
}

const BuildLayerRoot* LocalChunkStore::getLayer() const
{
    return &m_root;
}
}