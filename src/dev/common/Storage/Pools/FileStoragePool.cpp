#include <Engine.Storage/Pools/FileStoragePool.h>
#include <Engine.Base/String/String.h>
#include <Engine.Core/VirtualFileSystem.h>

#ifdef SvcCompilerMSVC
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

namespace rw
{
FileStoragePool::FileStoragePool(VirtualFileSystem* ioManager, const char* poolPath, const char* poolName)
    : m_ioManager(ioManager)
    , m_rootPath(poolPath)
{
    m_config.poolFormat = StorageFormat::File;
    m_config.poolPath = poolPath;
    m_config.poolName = poolName;
}

FileStoragePool::~FileStoragePool()
{
    // nop
}

void FileStoragePool::updateConfig(const StoragePoolConfig& config)
{
    // nop
}

void FileStoragePool::flush(bool trim)
{
    // nop
}

DbObject FileStoragePool::describe()
{
    PathStringBuilder tmpsb;
    const char* directoryName = m_ioManager->nativePathTranslation(m_config.poolPath, tmpsb);
    
    ULARGE_INTEGER freeBytes, totalBytes, totalFreeBytes;
    GetDiskFreeSpaceExA(directoryName, &freeBytes, &totalBytes, &totalFreeBytes);

    DbObjectBuilder builder;
    builder << "name" << m_config.poolName;
    builder << "diskType" << int(StorageFormat::File);
    builder << "diskPath" << m_config.poolPath;
    builder << "diskTotalBytes" << totalBytes.QuadPart;
    builder << "diskFreeBytes" << totalFreeBytes.QuadPart;
    return builder.done();
}

InsertResult FileStoragePool::putPatch(ContentId* digest, const ContentId& base, const SharedBuffer* chunk)
{
    return InsertResult::Error; // nop
}

InsertResult FileStoragePool::putPatch(ContentId* digest, const ContentId& base, const ContentId& delta)
{
    return InsertResult::Error; // nop
}

FetchResult FileStoragePool::getPatch(const ContentId& digest, ContentId& base, ContentId& delta)
{
    return FetchResult::NotFound;
}

InsertResult FileStoragePool::putChunk(const SharedBuffer* content)
{
    if(!content)
        return InsertResult::Error;

    ContentId digest;
    digest.calculate(content->bytes(), content->size64());

    StringBuilder<> path;
    makeKeyPath(digest, path);
    
    if(m_ioManager->fileExists(path.c_str()))
        return InsertResult::AlreadyPresent;

    ScopedRef fileBuf = m_ioManager->createFile(path.c_str());
    if(fileBuf.isNull())
        return InsertResult::Error;

    fileBuf->write(content->bytes(), content->size64());
    return InsertResult::Inserted;
}

FetchResult FileStoragePool::getChunk(const ContentId& digest, SharedBuffer** out)
{
    if(!out)
        return FetchResult::NoTargetSpecified;

    StringBuilder<> path;
    makeKeyPath(digest, path);

    *out = m_ioManager->shareFile(path.c_str());
    return (*out ? FetchResult::Found : FetchResult::NotFound);
}

FetchResult FileStoragePool::fetchChunk(const ContentId& digest, void* buf, u32* size)
{
    if(!buf || !size)
        return FetchResult::NoTargetSpecified;

    StringBuilder<> path;
    makeKeyPath(digest, path);
    ScopedRef buffer = m_ioManager->openFile(path.c_str());
    if(buffer.isNull())
        return FetchResult::NotFound;

    if(*size == NO)
    {
        *size = (u32)(buffer->getAvailableBytes());
    }

    if(buffer->read(buf, *size) < E_BIO_SUCCESS)
        return FetchResult::SysApiError;

    return FetchResult::Found;
}

bool FileStoragePool::hasChunk(const ContentId& digest)
{
    StringBuilder<> path;
    makeKeyPath(digest, path);
    return m_ioManager->fileExists(path.c_str());
}

bool FileStoragePool::dropChunks(const ContentId* digests, u32 count)
{
    if(!digests || !count)
        return false;

    for(u32 i = 0; i < count; i++)
    {
        StringBuilder<> path;
        makeKeyPath(digests[i], path);

        // already dropped, or doesn't exist at all
        if(!m_ioManager->fileExists(path.c_str()))
            continue;

        if(!m_ioManager->deleteFile(path.c_str()))
            return false;
    }

    return true;
}

void FileStoragePool::iterateChunks(ChunkCallback_t&& cb)
{
    
}

inline constexpr int kShardLevels = 3;
inline constexpr int kShardSize = 4;

void FileStoragePool::makeKeyPath(const ContentId& digest, StringEditor& outPath) const
{
    ContentIdStr s(digest);

    outPath << m_rootPath;

    for (int i = 0; i < kShardLevels; ++i)
	{
		outPath.appendRange(s.str + i * kShardSize, s.str + (i + 1) * kShardSize);
        outPath.appendChar('/');
	}

	outPath.appendChars(s.str + kShardLevels * 2);
}
}