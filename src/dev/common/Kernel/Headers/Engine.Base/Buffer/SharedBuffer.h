#pragma once
#include "../Bundle.h"
#include "../Crypto/SHA.h"
#include "../MIME.h"

namespace rw
{
class SharedBuffer
{
public:
    enum Flags : u16
    {
        kNone = 0,
        kOwned = 1 << 0,
        kReadOnly = 1 << 1
    };

    FB_BASE_API          SharedBuffer();
    FB_BASE_API virtual ~SharedBuffer();
    
    FB_BASE_API u16 retain();
    FB_BASE_API u16 release();

    FB_BASE_API virtual void ensureResources();
    FB_BASE_API virtual void releaseResources();
    FB_BASE_API virtual void setSize(StreamSize);
    
    FB_BASE_API MIME mimeType() const;
    FB_BASE_API void setMimeType(MIME);
    
    FB_BASE_API void* bytes() const;
    FB_BASE_API u32 size() const;
    FB_BASE_API StreamSize size64() const;

protected:
    void* m_data;
    StreamSize m_size;
    MIME m_mimeType;
    Flags m_flags;
    u16 m_refCount;
};

class MemorySharedBuffer : public SharedBuffer
{
public:
    FB_BASE_API          MemorySharedBuffer();
    FB_BASE_API          MemorySharedBuffer(u64 size);
    FB_BASE_API          MemorySharedBuffer(const void* data, u64 size, bool owns = false);
    FB_BASE_API virtual ~MemorySharedBuffer();

    virtual void ensureResources() override;
    virtual void releaseResources() override;
};

class MappedSharedBuffer : public SharedBuffer
{
public:
    FB_BASE_API          MappedSharedBuffer();
    FB_BASE_API          MappedSharedBuffer(const char* path, bool assert = false);
    FB_BASE_API virtual ~MappedSharedBuffer();

    virtual void ensureResources() override;
    virtual void releaseResources() override;
    virtual void setSize(StreamSize) override;

private:
    void* hFile;
    void* hMap;
};
}