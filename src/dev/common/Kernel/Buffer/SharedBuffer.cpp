#include <Engine.Base/Buffer/SharedBuffer.h>
#include <Engine.Base/Memory/VirtualAlloc.h>
#include <Engine.Base/Memory/Map.h>
#include <Engine.Base/Misc/Interlocked.h>
#include <cstring>

#ifdef SvcCompilerMSVC
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

namespace rw
{
SharedBuffer::SharedBuffer()
    : m_data(NIL)
    , m_size(NIL)
    , m_flags(SharedBuffer::kNone)
    , m_mimeType(Mime_Unknown)
    , m_refCount(0)
{
    retain();
}

SharedBuffer::~SharedBuffer()
{
    release();
}

u16 SharedBuffer::retain()
{
    return lock_inc(m_refCount);
}

u16 SharedBuffer::release()
{
    if(!m_refCount)
        return 0;

    if(!lock_dec(m_refCount))
    {
        releaseResources();
    }
    return m_refCount;
}

void SharedBuffer::ensureResources()
{
    // nop
}

void SharedBuffer::releaseResources()
{
    // nop
}

void SharedBuffer::setSize(StreamSize size)
{
    m_size = size;
}

MIME SharedBuffer::mimeType() const
{
    return m_mimeType;
}

void SharedBuffer::setMimeType(MIME m)
{
    m_mimeType = m;
}

void* SharedBuffer::bytes() const
{
    SharedBuffer* self = const_cast<SharedBuffer*>(this);
    if(MappedSharedBuffer* asMapped = dynamic_cast<MappedSharedBuffer*>(self))
        asMapped->ensureResources();
    
    return m_data;
}

u32 SharedBuffer::size() const
{
    return static_cast<u32>(m_size);
}

StreamSize SharedBuffer::size64() const
{
    return m_size;
}

MemorySharedBuffer::MemorySharedBuffer() = default;

MemorySharedBuffer::MemorySharedBuffer(u64 size)
{
    EASSERTF(size, "Memory buffer with a size of zero could not be created");

    m_data = HeapAllocGlobal(size);
    m_size = size;
    m_mimeType = Mime_Unknown;
    m_flags = SharedBuffer::kOwned;
}

MemorySharedBuffer::~MemorySharedBuffer()
{
    releaseResources();
}

MemorySharedBuffer::MemorySharedBuffer(const void* data, u64 size, bool owns)
{
    EASSERTF(data, "Memory buffer with a null pointer could not be created");
    EASSERTF(size, "Memory buffer with a size of zero could not be created");

    m_data = HeapAllocGlobal(size);
    std::memcpy(m_data, data, size);
    m_size = size;
    m_mimeType = Mime_Unknown;
    m_flags = owns ? SharedBuffer::kOwned : SharedBuffer::kReadOnly;
}

void MemorySharedBuffer::ensureResources()
{
    // nop
}

void MemorySharedBuffer::releaseResources()
{
    if(m_data && (m_flags & SharedBuffer::kOwned))
    {
        HeapFreeGlobal(m_data);
        m_data = NIL;
    }
}

MappedSharedBuffer::MappedSharedBuffer()
    : SharedBuffer()
    , hFile(INVALID_HANDLE_VALUE)
    , hMap(NIL)
{}

MappedSharedBuffer::MappedSharedBuffer(const char* path, bool assert)
{
    m_data = NIL;
    m_size = NIL;
    m_mimeType = Mime_Unknown;
    m_flags = SharedBuffer::kOwned;
    
    hFile = CreateFileA(LPCSTR(path), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NIL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NIL);
    if(assert)
        EASSERTF(hFile != INVALID_HANDLE_VALUE, "Failed to open file handle to %s", path);
    
    LARGE_INTEGER fileSize;
    fileSize.QuadPart = 0;
    
    GetFileSizeEx(hFile, &fileSize);
    m_size = fileSize.QuadPart;
    hMap = NIL;
}

MappedSharedBuffer::~MappedSharedBuffer()
{
    releaseResources();
}

void MappedSharedBuffer::ensureResources()
{
    if(hFile == INVALID_HANDLE_VALUE)
    {
        return;
    }

    if(hMap)
    {
        return;
    }

    hMap = CreateFileMappingA(hFile, NIL, PAGE_READWRITE, 0, 0, NIL); // private mapping
    
    if(!hMap)
    {
        EERROR("Failed to create file mapping");        
        releaseResources();
        return;
    }
    
    m_data = MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    EASSERT(m_data, "Failed to map file");
}

void MappedSharedBuffer::releaseResources()
{
    //ELOGF("Releasing mapped resources...");

    if(m_data)
    {
        UnmapViewOfFile(m_data);
        m_data = NIL;
    }

    if(hMap)
    {
        CloseHandle(hMap);
        hMap = NIL;
    }

    if(hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }
}

void MappedSharedBuffer::setSize(StreamSize size)
{
    BOOL bResult;
    LARGE_INTEGER lDistanceToMove;    
    
    if(hFile == INVALID_HANDLE_VALUE || !m_data)
        return;

    lDistanceToMove.QuadPart = size;
    bResult = SetFilePointerEx(hFile, lDistanceToMove, NIL, FILE_BEGIN);
    if(!bResult)
        return;

    bResult = SetEndOfFile(hFile);
    if(!bResult)
        return;

    bResult = UnmapViewOfFile(m_data);
    if(!bResult)
        return;

    m_data = MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if(!m_data)
        return;

    SharedBuffer::setSize(size);
}
}