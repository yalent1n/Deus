#include <Engine.Storage/Pools/FlexibleStoragePool.h>
#include <Engine.Storage/Delta.h>
#include <Engine.Base/String/String.h>
#include <Engine.Base/HashSet.h>
#include <cstring>

#ifdef SvcCompilerMSVC
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

namespace rw
{
static inline const char s_schemaVersionBytes[] = {'\x69','\x24','\x73','\x57','\xE9','\x46','\x4B','\x08','\x80','\xB4','\xD9','\x71','\x4F','\x22','\xF1','\xED'};
const Guid CasCatalogHeader::SchemaVersion = Guid((u8*)&s_schemaVersionBytes[0]);

static inline const char s_encryptedSchemaVersionBytes[] = {'\x02','\x9F','\x86','\xAE','\x6E','\xA6','\x47','\xC6','\xA9','\x9E','\x49','\xCD','\x92','\x23','\x5A','\x0D'};
const Guid CasCatalogHeader::EncryptedSchemaVersion = Guid((u8*)&s_encryptedSchemaVersionBytes[0]);

enum FileState : u8
{
    FileState_Unknown,
    FileState_Verified,     // from the catalog header
    FileState_Pending,     // currently pending
    FileState_Cached
};

bool chunk_find_op(
    const ContentId& digest,
    const Vec<CatEntry>& files,
    const CasStoragePool::Clusters_t& clusters,
    const Vec<CasStoragePool::PendingWrite>& pendingWrites,
    FileState& fileState) 
{
    const auto pendingSearch = [&](const CasStoragePool::PendingWrite& w) -> bool
    {
        const CatEntry& f = files[w.fileIndex];
        return memcmp(&f.sha, &digest, sizeof digest) == 0;
    };

    const auto fileSearch = [&](const CatEntry& f) -> bool
    {
        return memcmp(&f.sha, &digest, sizeof digest) == 0;
    };

    if(Vec<CasStoragePool::PendingWrite>::const_iterator pendingIt = pendingWrites.find_if(pendingSearch); 
        pendingIt != pendingWrites.end())
    {
        fileState = FileState_Pending;
        return true;
    }

    if(Vec<CatEntry>::const_iterator fileIt = files.find_if(fileSearch); 
        fileIt != files.end())
    {
        fileState = FileState_Verified;
        return true;
    }

    for(const auto& it : clusters)
    {
        const CasStoragePool::Cluster& cluster = it.second;
        if(cluster.chunks.find(digest) != cluster.chunks.end())
        {
            fileState = FileState_Cached;
            return true;
        }
    }

    fileState = FileState_Unknown;
    return false;
}

bool chunk_find_patch(
    const ContentId& digest,
    const Vec<CatPatchEntry>& files,
    FileState& fileState) 
{
    const auto fileSearch = [&](const CatPatchEntry& f) -> bool
    {
        return memcmp(&f.sha, &digest, sizeof digest) == 0;
    };

    if(Vec<CatPatchEntry>::const_iterator fileIt = files.find_if(fileSearch); 
        fileIt != files.end())
    {
        fileState = FileState_Verified;
        return true;
    }

    fileState = FileState_Unknown;
    return false;
}

CasStoragePool::CasStoragePool(VirtualFileSystem* vfs, const char* poolPath, const char* poolName)
    : m_vfs(vfs)
    , m_dir(poolPath)
    , m_clusterDir(nullptr)
    , m_clusterId(0)
    , m_cluster(nullptr)
{
    // BIG TODO: POPULATE ON DEMAND

    StringBuilder<> clusterDir;
    clusterDir << poolPath;
    if(!issep(clusterDir.back()))
        clusterDir << '/';
    EASSERTF(m_vfs->createDirectory(clusterDir.c_str()), "Couldn't create CAS directory '%s'", clusterDir.c_str());
    m_clusterDir = m_dir = clusterDir.detach();

    StringBuilder<> filter;
    filter << m_clusterDir << "*.*";

    FileIterator* iterator = vfs->getFileIterator(filter.c_str());
    if(iterator)
    {
        char name[_MAX_PATH];

        FileInfo file;
        while(iterator->getNextFile(name, _MAX_PATH, &file))
            ;

        if(!isnil(name))
        {            
            u32 id = u32(atoi(name));
            m_clusterId = id;
        }
    }
    else
    {
        EERRORF("No iterator for CLUSTER directory %s", m_clusterDir);
    }

    StringBuilder<> catalogFile;
    catalogFile << m_clusterDir << "cas.cat";
    m_catalog.stream = m_vfs->createFile(c_str(catalogFile));
    EASSERT(m_catalog.stream, "Failed to create CAT file");
    
    m_config.poolPath = m_clusterDir;
    m_config.poolName = poolName;
    m_config.poolFormat = StorageFormat::Flexible;

    readCatalog();
    createCluster(m_clusterId);
    EDEBUGF("%s - Bootstrapped", poolName);
}

CasStoragePool::~CasStoragePool()
{
    if(m_cluster)
    {
        delete m_cluster->stream;
        m_cluster->stream = nullptr;
        m_cluster = nullptr;
    }

    if(m_catalog.stream)
    {
        delete m_catalog.stream;
        m_catalog.stream = nullptr;
    }
}

void CasStoragePool::updateConfig(const StoragePoolConfig& config)
{
    if(m_config.clusterSizeMB != config.clusterSizeMB)
        m_config.clusterSizeMB = config.clusterSizeMB;

    // TODO: if pool path differs create new m_dir and m_clusterDir the same way as in the constructor
}

void CasStoragePool::flush(bool trim)
{    
    CriticalSectionScope cs(m_cs);

    ssize_t beforeTrimCount = m_catalog.header.entryCount;

    if(trim)
        fullTrim();

    ssize_t afterTrimCount = m_catalog.header.entryCount;
    ssize_t deletedCount = (beforeTrimCount - afterTrimCount);

    for(const PendingWrite& write : m_pendingWrites)
    {
        CatEntry* file = m_catalog.files.begin() + write.fileIndex;
        (void) appendChunk(write.cluster, write.data, file);
        file->flagsAndCluster &= ~CatEntry::Pending;
    }    
    
    m_pendingWrites.clear();

    ssize_t afterWriteCount = m_catalog.files.size();
    ssize_t addedCount = afterWriteCount - (m_catalog.header.entryCount);

    // Flush every cluster, instead of doing it for every write after
    for(auto& it : m_clusters)
    {
        Cluster& cluster = it.second;
        if(cluster.stream)
        {
            cluster.stream->flush();
        }
    }

    EDEBUGF("%s - Added %ld new files and removed %ld unneccesary files (%u files in total)", m_config.poolName, addedCount, deletedCount, afterWriteCount);
    m_catalog.header.entryCount = m_catalog.files.size();
    m_catalog.header.patchedEntryCount = m_catalog.patchFiles.size();

    // update state
    writeCatalog();
}

DbObject CasStoragePool::describe()
{
    PathStringBuilder tmpsb;
    const char* directoryName = m_vfs->nativePathTranslation(m_config.poolPath, tmpsb);

    ULARGE_INTEGER freeBytes, totalBytes, totalFreeBytes;
    GetDiskFreeSpaceExA(directoryName, &freeBytes, &totalBytes, &totalFreeBytes);

    DbObjectBuilder builder;
    builder << "name" << m_config.poolName;
    builder << "diskType" << int(StorageFormat::Flexible);
    builder << "diskPath" << m_config.poolPath;
    builder << "diskTotalBytes" << m_catalog.header.totalBytesUsed;
    builder << "diskFreeBytes" << totalFreeBytes.QuadPart - m_catalog.header.totalBytesUsed;
    return builder.done();
}

InsertResult CasStoragePool::appendChunk(const Cluster* extent, const SharedBuffer* data, CatEntry* file)
{
    if(!data || !extent || !file)
        return InsertResult::Error;

    Buffer* stream = extent->stream;

    if(stream->write(data->bytes(), data->size()) < E_BIO_SUCCESS)
        return InsertResult::Error;
    
    StreamSize pos = stream->getPosition();
    StreamSize newExtentSize = pos + (m_config.alignment - (pos % m_config.alignment)) % m_config.alignment;
    stream->setPosition(newExtentSize);
    m_catalog.header.totalBytesUsed += data->size();

    // not pending anymore
    file->flagsAndCluster &= ~CatEntry::Pending;
    //EDEBUGF("CAS write (%p, %zuB)", data->bytes(), data->size64());
    return InsertResult::Inserted;
}

FetchResult CasStoragePool::readChunk(const CatEntry& file, SharedBuffer** out)
{
    if(!out)
        return FetchResult::NoTargetSpecified;

    if(file.isErased())
        return FetchResult::NotFoundCatalog;

    *out = nullptr;

    PendingWrites_t::iterator pendingIt = m_pendingWrites.find_if([&](const PendingWrite& w) -> bool
    {
        return w.fileIndex == u32(&file - m_catalog.files.begin());
    });
    if(pendingIt) 
    {
        *out = const_cast<SharedBuffer*>(pendingIt->data);
        return FetchResult::FoundCold;
    }

    Cluster& extent = openCluster(file.cluster());
    
    Cluster::ChunkMap_t::iterator foundIt = extent.chunks.find(file.sha);
    if(foundIt != extent.chunks.end())
    {
        *out = const_cast<SharedBuffer*>(foundIt->second);
        return FetchResult::FoundHot;
    }

    Buffer* stream = extent.stream;

    u32 prevPos = stream->getPosition();
        
    stream->setPosition(file.offset);
    SharedBuffer* chunk = new MemorySharedBuffer(file.size);
    if(stream->read(chunk->bytes(), chunk->size()) < E_BIO_SUCCESS) // failed to read from extent
        return FetchResult::SysApiError;

    stream->setPosition(prevPos);
    *out = chunk;
    return FetchResult::Found;
}

FetchResult CasStoragePool::readChunkPatched(const CatEntry& base, const CatEntry& delta, SharedBuffer** out)
{
    if(!out)
        return FetchResult::NoTargetSpecified;

    *out = nullptr;

    FetchResult fetchResult;
    SharedBuffer* baseData;
    SharedBuffer* deltaData;
    
    fetchResult = readChunk(base, &baseData);
    if(fetchResult < FetchResult::Found)
        return FetchResult::BaseNotFound;
        
    fetchResult = readChunk(delta, &deltaData);
    if(fetchResult < FetchResult::Found)
        return FetchResult::DeltaNotFound;

    CompressionHelper compressor(CodecMethod_ZStd);
    FluxPatchEncoder decoder(compressor);

    AllocatingBuffer headData;
    decoder.decodeDelta(baseData->bytes(), baseData->size(), deltaData->bytes(), deltaData->size(), headData);

    *out = new MemorySharedBuffer(headData.detach(), headData.length(), true);
    return FetchResult::Found;
}

bool CasStoragePool::readChunk(const CatEntry& file, void* buf, u32* size)
{
    if(file.isErased())
        return false;

    if(!size)
        return false;

    PendingWrites_t::iterator pendingIt = m_pendingWrites.find_if([&](const PendingWrite& w) -> bool
    {
        return w.fileIndex == u32(&file - m_catalog.files.begin());
    });
    if(pendingIt)
    {
        const SharedBuffer* chunk = pendingIt->data;
        if(*size > chunk->size() || *size == NO)
            *size = chunk->size();

        if(buf)
            memcpy(buf, chunk->bytes(), *size);
        
        return true;
    }

    Cluster& extent = openCluster(file.cluster());
    
    Cluster::ChunkMap_t::iterator foundIt = extent.chunks.find(file.sha);
    if(foundIt != extent.chunks.end())
    {
        const SharedBuffer* chunk = foundIt->second;
        if(*size > chunk->size() || *size == NO)
            *size = chunk->size();

        if(buf)
            memcpy(buf, chunk->bytes(), *size);

        return true;
    }

    if(*size > file.size || *size == NO)
        *size = file.size;

    if(buf)
    {
        Buffer* stream = extent.stream;
        u32 prevPos = stream->getPosition();
        stream->setPosition(file.offset);
    
        size_t readSize = *size;
        if(stream->read(buf, readSize) < E_BIO_SUCCESS) // failed to read from extent
            return false;
    
        stream->setPosition(prevPos);
    }

    return true;
}

InsertResult CasStoragePool::putPatch(ContentId* digest, const ContentId& base, const ContentId& delta)
{
    if(!digest)
        return InsertResult::NoSourceSpecified;

    digest->calculateHmac(delta.hash, sizeof(delta.hash), base.hash, sizeof(base.hash));

    FileState dmy;
    if(chunk_find_patch(*digest, m_catalog.patchFiles, dmy))
    {
        return InsertResult::AlreadyPresent;
    }

    // add to patch file list
    CatPatchEntry& file = m_catalog.patchFiles.push_back();
    file.sha = *digest;
    file.base = base;
    file.delta = delta;

    return InsertResult::Inserted;
}

InsertResult CasStoragePool::putPatch(ContentId* digest, const ContentId& base, const SharedBuffer* head)
{
    InsertResult ierr;
    FetchResult ferr;

    if(!digest || !head)
        return InsertResult::NoSourceSpecified;

    SharedBuffer* baseBuffer;
    ferr = getChunk(base, &baseBuffer);
    if(ferr < FetchResult::Found)
        return InsertResult::BaseNotFound;

    DeltaOps_t ops;
    
    DeltaGenerator deltaGenerator;
    if(!deltaGenerator.computeDelta(baseBuffer->bytes(), baseBuffer->size(), head->bytes(), head->size(), ops))
        return InsertResult::DeltaNotComputed;

    CompressionHelper compressor(CodecMethod_ZStd);
    FluxPatchEncoder encoder(compressor);

    AllocatingBuffer buf;
    encoder.encodeDelta(baseBuffer->bytes(), baseBuffer->size(), head->bytes(), head->size(), ops, buf);

    ContentId delta;
    delta.calculate(buf.bytes(), buf.length());

    SharedBuffer* deltaBuffer = new MemorySharedBuffer(buf.detach(), buf.length(), true);
    deltaBuffer->setMimeType(Mime_Patch);

    ierr = putChunk(deltaBuffer);
    if(ierr == InsertResult::AlreadyPresent)
        return ierr;

    if(ierr >= InsertResult::Error)
        return ierr;

    return putPatch(digest, base, delta);
}

FetchResult CasStoragePool::getPatch(const ContentId& digest, ContentId& base, ContentId& delta)
{
    const auto patchSearch = [&](const CatPatchEntry& f) -> bool
    {
        return memcmp(&f.sha, &digest, sizeof(ContentId)) == 0;
    };

    if(Vec<CatPatchEntry>::const_iterator fileIt = m_catalog.patchFiles.find_if(patchSearch); 
        fileIt != m_catalog.patchFiles.end())
    {
        base = fileIt->base;
        delta = fileIt->delta;
        return FetchResult::Found;
    }

    return FetchResult::NotFound;
}

InsertResult CasStoragePool::putChunk(const SharedBuffer* chunk)
{
    if(!chunk)
        return InsertResult::NoSourceSpecified;
        
    ContentId digest;
    digest.calculate(chunk->bytes(), chunk->size64());

    FileState dmy;
    if(chunk_find_op(digest, m_catalog.files, m_clusters, m_pendingWrites, dmy))
    {
        return InsertResult::AlreadyPresent;
    }

    makeWritableCluster(chunk->size64());

    // add to chunk map
    m_cluster->chunks.insert(digest, chunk);

    // add to file list
    u32 fileIndex = m_catalog.files.size();
    CatEntry& file = m_catalog.files.push_back();
    file.sha = digest;
    file.offset = m_cluster->usedBytes;
    file.size = chunk->size();
    file.flagsAndCluster |= CatEntry::Pending;
    file.setCluster(m_clusterId);

    if(chunk->mimeType() == Mime_Patch)
    {
        file.flagsAndCluster |= CatEntry::Delta;
    }
    if(chunk->mimeType() == Mime_Compressed)
    {
        file.flagsAndCluster |= CatEntry::Compressed;
    }

    // A: extent's used bytes is used as a cursor
    // B: storage header's used bytes is used how much bytes are used in the actual pool
    m_cluster->usedBytes += ((chunk->size() + m_config.alignment - 1) / m_config.alignment) * m_config.alignment;

    CriticalSectionScope cs(m_cs);
    m_pendingWrites.push_back({m_cluster, fileIndex, chunk});
    return InsertResult::Inserted;
}

FetchResult CasStoragePool::getChunk(const ContentId& digest, SharedBuffer** out)
{
    if(!out)
        return FetchResult::NoTargetSpecified;

    *out = nullptr;

    // TOOD:
    // either get chunk from extent map, or cache from the containing extent.
    // if the catalog entry list contains the chunk, cache it into the extent map.

    const CatEntry* file = m_catalog.files.find_if([&](const CatEntry& file)
    {
        return memcmp(&file.sha, &digest, sizeof(ContentId)) == 0;
    });

    if(file == m_catalog.files.end())
    {
        const CatPatchEntry* patchFile = m_catalog.patchFiles.find_if([&](const CatPatchEntry& file)
        {
            return memcmp(&file.sha, &digest, sizeof(ContentId)) == 0;
        });
        
        if(patchFile != m_catalog.patchFiles.end())
        {
            const CatEntry* baseFile = m_catalog.files.find_if([&](const CatEntry& file)
            {
                return memcmp(&patchFile->base, &file.sha, sizeof(file.sha)) == 0;
            });
            if(baseFile == m_catalog.files.end())
                return FetchResult::BaseNotFound;

            const CatEntry* deltaFile = m_catalog.files.find_if([&](const CatEntry& file)
            {
                return memcmp(&patchFile->delta, &file.sha, sizeof(file.sha)) == 0;
            });
            if(deltaFile == m_catalog.files.end())
                return FetchResult::DeltaNotFound;

            return readChunkPatched(*baseFile, *deltaFile, out);
        }
        
        // const CatCryptoEntry* cryptoFile = m_catalog.encryptedFiles.find_if([&](const CatCryptoEntry& file)
        // {
        //     return memcmp(&file.cat.sha, &digest, sizeof digest) == 0;
        // });

        // TODO: 
        // if(cryptoFile)
        //     return readChunkEncrypted(cryptoFile->cat, cryptoFile->keyId, cryptoFile->checkSum);
        
        return FetchResult::NotFound;
    }

    return readChunk(*file, out);
}

bool CasStoragePool::hasChunk(const ContentId& digest)
{    
    FileState dummy;
    return chunk_find_op(digest, m_catalog.files, m_clusters, m_pendingWrites, dummy);
}

bool CasStoragePool::dropChunks(const ContentId* digests, u32 count)
{
    if(digests && count)
    {
        for(u32 i = 0; i < count; ++i)
        {
            const ContentId& digest = digests[i];

            Cluster::ChunkMap_t::iterator cache = Cluster::ChunkMap_t::iterator();
            Vec<CatEntry>::iterator file = nullptr;
            PendingWrites_t::iterator pending = nullptr;

            FileState fileState;
            // nowhere else to be found
            if(!chunk_find_op(digest, m_catalog.files, m_clusters, m_pendingWrites, fileState))
            {                
                return false;
            }
            if(fileState != FileState_Verified && fileState != FileState_Pending)
                return false;

            file = m_catalog.files.find_if([&](const CatEntry& f)
            {
                return memcmp(&f.sha, &digest, sizeof f.sha) == 0;
            });
            if(!file)
                return false;            

            u32 fileIndex = u32(file - m_catalog.files.begin());
            Cluster& owningCluster = openCluster(file->cluster());

            if(file->isErased())
            {
                owningCluster.erasedFiles.push_back({cache, fileIndex, pending});
                continue;
            }

            file->flagsAndCluster |= CatEntry::Erased;
            m_catalog.header.totalBytesCleanup += file->size;

            // NOTE
            // Do you see how this ordered operations look like a state machine?
            // Like, first time putting a chunk makes a pending -and catalog file
            // and adding it to the extent's chunks.
            // This is why the code eliminates the first one (pending), and the second
            // one after (persistent catalog file), and lastly if this was cached, remove it.

            // added now (omits the CAS write)
            if(fileState == FileState_Pending)
            {
                pending = m_pendingWrites.find_if([&](const PendingWrite& w)
                {
                    const CatEntry& f = m_catalog.files[w.fileIndex];
                    return memcmp(&f.sha, &digest, sizeof digest) == 0;
                });

                if(!pending)
                {
                    EERROR("pending write not found (should have been)");
                    return false;
                }
            }            
            
            for(Clusters_t::pair_type& it : m_clusters)
            {
                Cluster& cluster = it.second;
                Cluster::ChunkMap_t::iterator chunkIt = cluster.chunks.find(digest);

                if(chunkIt != cluster.chunks.end())
                {
                    //Log("Found in cache");
                    cache = chunkIt;
                }                    
            }
            
            owningCluster.erasedFiles.push_back({cache, fileIndex, pending});
        }

        // actual removal is not done yet (till next flush)
        // so update the catalog file (refresh state)
        writeCatalog();

        return true;
    }

    return false;
}

bool CasStoragePool::collectGarbage()
{
    // nothing to clean up
    if(m_catalog.header.totalBytesCleanup == 0)
        return true;

    for(const CatEntry& file : m_catalog.files)
    {
        if(file.isErased())
        {
            u32 fileIndex = u32(&file - m_catalog.files.begin());
            Cluster& extent = openCluster(file.cluster());

            Cluster::ErasedList_t::iterator erasedIt = extent.erasedFiles.find_if([&](const Cluster::ErasedFile& predErase) -> bool 
            {                 
                return predErase.fileIndex == fileIndex;
            });
            if(erasedIt) // already marked for erase
                continue;

            Cluster::ErasedFile& erased = extent.erasedFiles.push_back();

            erased.cache = Cluster::ChunkMap_t::iterator();
            erased.fileIndex = fileIndex;
            erased.pending = nullptr; // shouldn't exist as a pending write because it exists already
    
            Cluster::ChunkMap_t::iterator cacheIt = extent.chunks.find(file.sha);
            if(cacheIt != extent.chunks.end())
            {
                erased.cache = cacheIt;
            }
        }
    }

    return true;
}

void CasStoragePool::iterateChunks(ChunkCallback_t&& cb)
{
    for(const CatEntry& file : m_catalog.files)
    {
        if(!file.isErased())
        {
            cb(file.sha);
        }
    }
}

bool CasStoragePool::partTrim(u32 id)
{
    // NOTE: this is a bit lossy, because this just reduces the size of the cluster, but didn't remove the catalog link
    Cluster& cluster = openCluster(id);
    if(cluster.erasedFiles.empty())
        return true;
    
    EDEBUGF("%s - Cluster %u has %d files in total to erase", m_config.poolName, cluster.id, cluster.erasedFiles.size());
    trimClusterSpace(cluster);
    return true;
}

bool CasStoragePool::fullTrim()
{
    if(!collectGarbage())
    {
        return false;
    }

    for(auto& it : m_clusters)
    {
        Cluster& cluster = it.second;
        if(cluster.erasedFiles.empty())
            continue;
        
        EDEBUGF("%s - Cluster %u has %d files in total to erase", m_config.poolName, cluster.id, cluster.erasedFiles.size());
        trimClusterSpace(cluster);
    }

    // HERE do the actual trim of the catalog file
    Vec<CatEntry> filteredFiles;
    for(const CatEntry& f : m_catalog.files)
    {
        if(f.isErased())
            continue;

        filteredFiles.push_back(f);
    }

    if(filteredFiles.empty())
        return true;

    m_catalog.files.set_size(filteredFiles.size());
    m_catalog.files.set_data(filteredFiles.detach());
    // TODO: create new cas.cat in m_dir, don't update the "temporary" catalog file
    writeCatalog();
    return true;
}

CasStoragePool::Cluster& CasStoragePool::openCluster(u32 id)
{
    Cluster& cluster = m_clusters.insert(id).second;
    if(cluster.stream)
        return cluster;
        
    cluster.id = id;
    cluster.openStream(*this);
    return cluster;
}

CasStoragePool::Cluster& CasStoragePool::createCluster(u32 id)
{      
    Cluster& cluster = openCluster(id);
    m_cluster = &cluster;
    return cluster;
}

void CasStoragePool::makeWritableCluster(u32 size)
{
    StreamSize chunkEnd = (StreamSize) m_cluster->usedBytes + size;
    StreamSize clusterEnd = (size_t)m_config.clusterSizeMB MiB;
    if(chunkEnd > clusterEnd)
    {
        createCluster(++m_clusterId);
    }
}

void CasStoragePool::trimClusterSpace(Cluster& cluster)
{
    EASSERTF(cluster.stream, "%s - Cluster %u is not opened", m_config.poolName, cluster.id);
    FormattedStringBuilder<_MAX_PATH> origClusterFile("%s%02d", m_clusterDir, cluster.id);
    FormattedStringBuilder<_MAX_PATH> tempClusterFile("%s%02d.tmp", m_clusterDir, cluster.id);
    
    // close file
    cluster.closeStream();

    // rename current to temporary
    if(!m_vfs->moveFile(c_str(origClusterFile), c_str(tempClusterFile)))
    {
        return;
    }

    cluster.stream = m_vfs->createFile(c_str(tempClusterFile), true);
    EASSERTF(cluster.stream, "Failed to create temporary file handle for cluster %u", cluster.id);

    // NOTE:
    // Don't erase the catalog link, simply remove the content.
    for(Cluster::ErasedFile& erased : cluster.erasedFiles)
    {
        const CatEntry& f = m_catalog.files[erased.fileIndex];
        __builtin_assert(f.isErased());

        if(erased.pending)
        {
            CriticalSectionScope cs(m_cs);
            (void) m_pendingWrites.erase(erased.pending);
            erased.pending = nullptr;
        }

        if(erased.cache != Cluster::ChunkMap_t::iterator())
        {
            (void) cluster.chunks.erase(f.sha);
            erased.cache = Cluster::ChunkMap_t::iterator();
        }        

        m_catalog.header.totalBytesUsed -= f.size;
        m_catalog.header.totalBytesCleanup -= f.size;
    }

    // create cluster without erased files
    Buffer* originalStream = m_vfs->createFile(c_str(origClusterFile), true);
    EASSERTF(originalStream, "Failed to create stream (after garbage collection in cluster %u)", cluster.id);

    // collection leftover entries which are not erased, can be a heavy task
    Vec<CatEntry*> writeFiles;
    for(CatEntry& file : m_catalog.files)
    {
        if(file.cluster() == cluster.id && !file.isErased())
            writeFiles.push_back(&file);
    }

    algorithm::insertion_sort(writeFiles.begin(), writeFiles.end(), [&](const CatEntry* a, const CatEntry* b) -> bool
    {
        return a->offset < b->offset;
    });    

    // write "new" files to cluster
    for(CatEntry* file : writeFiles)
    {
        SharedBuffer* chunk;
        FetchResult err = readChunk(*file, &chunk);
        if(err < FetchResult::Found)
        {
            EERRORF("Failed to read chunk %s from (old) cluster %u", ContentIdStr(file->sha).str, cluster.id);
            continue;
        }

        file->offset = (u32) originalStream->getPosition();

        if(!originalStream->write(chunk->bytes(), chunk->size()))
        {
            EERRORF("Failed to write chunk %s to (new) cluster %u", ContentIdStr(file->sha).str, cluster.id);
            continue;
        }
        
        StreamSize newClusterSize = (((file->offset + file->size) + m_config.alignment - 1) / m_config.alignment) * m_config.alignment;
        originalStream->setPosition(newClusterSize);
        //EDEBUGF("CAS write %s %uB in %02x at %u", ContentIdStr(file->sha).str, file->size, cluster.id, file->offset);
    }
    originalStream->flush();
    
    m_catalog.header.entryCount -= cluster.erasedFiles.size();
    cluster.erasedFiles.clear();

    // reopen file 
    delete cluster.stream;
    m_vfs->deleteFile(c_str(tempClusterFile));
    cluster.stream = originalStream;
    cluster.usedBytes = originalStream->getAvailableBytes();

    // update catalog links
    writeCatalog();
}

void CasStoragePool::readCatalog()
{
    if(!m_catalog.stream)
        return;

    if(!m_catalog.stream->getAvailableBytes())
    {
        std::memcpy(&m_catalog.header.schemaVersion, &CasCatalogHeader::SchemaVersion, sizeof m_catalog.header.schemaVersion);
        m_catalog.header.totalBytesUsed = 0;
        m_catalog.header.totalBytesCleanup = 0;
        m_catalog.header.entryCount = 0;
        m_catalog.header.patchedEntryCount = 0;
        m_catalog.header.encryptedEntryCount = 0;
        return;    
    }
    
    if(m_catalog.stream->read(&m_catalog.header, sizeof m_catalog.header) < E_BIO_SUCCESS)
        return;
    
    if(m_catalog.header.schemaVersion != CasCatalogHeader::SchemaVersion)
        return;

    m_catalog.files.reserve(m_catalog.header.entryCount);
    if(m_catalog.stream->read(m_catalog.files.begin(), m_catalog.header.entryCount * sizeof(CatEntry)) < E_BIO_SUCCESS)
        return;
    m_catalog.files.set_size(m_catalog.header.entryCount);
    
    m_catalog.patchFiles.reserve(m_catalog.header.patchedEntryCount);
    if(m_catalog.stream->read(m_catalog.patchFiles.begin(), m_catalog.header.patchedEntryCount * sizeof(CatPatchEntry)) < E_BIO_SUCCESS)
        return;
    m_catalog.patchFiles.set_size(m_catalog.header.patchedEntryCount);
    
    m_catalog.encryptedFiles.reserve(m_catalog.header.encryptedEntryCount);
    if(m_catalog.stream->read(m_catalog.encryptedFiles.begin(), m_catalog.header.encryptedEntryCount * sizeof(CatCryptoEntry)) < E_BIO_SUCCESS)
        return;
    m_catalog.encryptedFiles.set_size(m_catalog.header.encryptedEntryCount);

    //EDEBUGF("%s - read %u entries (total bytes used: %u, total bytes to clean: %u)", m_config.poolName, m_catalog.header.entryCount, m_catalog.header.totalBytesUsed, m_catalog.header.totalBytesCleanup);
}

void CasStoragePool::writeCatalog()
{
    m_catalog.stream->seek(0, E_BIO_SEEK_START);
    m_catalog.stream->write(&m_catalog.header, sizeof m_catalog.header);
    m_catalog.stream->write(m_catalog.files.begin(), m_catalog.files.size() * sizeof(CatEntry));
    m_catalog.stream->write(m_catalog.patchFiles.begin(), m_catalog.patchFiles.size() * sizeof(CatPatchEntry));
    m_catalog.stream->write(m_catalog.encryptedFiles.begin(), m_catalog.encryptedFiles.size() * sizeof(CatCryptoEntry));
    //ELOGF("Entry count at state update: %u", m_catalog.header.entryCount);
}
}