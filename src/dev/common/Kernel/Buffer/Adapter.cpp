#include <Engine.Base/Buffer/Adapter.h>
#include <Engine.Base/Memory/VirtualAlloc.h>
#include <cstring>
#include <stdio.h>

namespace rw
{
inline constexpr size_t kDefaultAdapterSize = 128;
inline constexpr size_t kDefaultAdapterAlignment = 16;

Adapter::Adapter(const void* rodata, size_t rolen, OwnershipPolicy ownership)
    : m_bytes((u8*) rodata)
    , m_end((u8*) rodata + rolen)
    , m_position(0)
    , m_length(rolen)
    , m_ownership(ownership)
    , m_isDynamic(false)
{
    m_access = BIOACCESS(E_BIO_READ | E_BIO_SEEK | E_BIO_FLUSH | E_BIO_WRITE);
}

Adapter::Adapter(bool allocSpace)
    : m_bytes(nullptr)
    , m_end(nullptr)
    , m_position(0)
    , m_length(0)
    , m_ownership(allocSpace ? kAssumeOwnership : kNoOwnership)
    , m_isDynamic(true)
{
    if(allocSpace)
    {
        m_bytes = (u8*) HeapAllocGlobal(kDefaultAdapterSize, kDefaultAdapterAlignment);
        m_end = m_bytes + kDefaultAdapterSize;
    }
    
    m_access = allocSpace ? BIOACCESS(E_BIO_WRITE | E_BIO_READ | E_BIO_SEEK | E_BIO_FLUSH) : BIOACCESS(0);
}

const u8* Adapter::bytes() const
{
    return m_bytes;
}

const u8* Adapter::detach() const
{
    const u8* bytes = m_bytes;
    const_cast<Adapter*>(this)->m_bytes = nullptr;
    return bytes;
}

SharedBuffer* Adapter::detachSharedBuffer() const
{
    const u8* bytes = m_bytes;
    const_cast<Adapter*>(this)->m_bytes = nullptr;
    return new MemorySharedBuffer(bytes, m_length, true);
}

const u8* Adapter::marker() const
{
    return m_bytes + m_position;
}

const size_t Adapter::position() const
{
    return (const size_t)(m_position);
}

const size_t Adapter::length() const
{
    return (const size_t)(m_length);
}

OwnershipPolicy Adapter::ownershipPolicy() const
{
    return OwnershipPolicy(m_ownership);
}

void* Adapter::allocateWrite(size_t writeAmount)
{
    static constexpr char _dummy[] = "";
    if(m_bytes && (m_access & E_BIO_WRITE))
    {
        size_t BeginPos = m_position;        
        for(size_t i = 0; i < writeAmount; i++)
        {
            if(write(_dummy, 1) < E_BIO_SUCCESS)
                return NIL;
        }

        return m_bytes + BeginPos;
    }

    return NIL;
}

void Adapter::flush()
{
    if(m_bytes && (ownershipPolicy() & kAssumeOwnership))
    {
        EASSERT(HeapFreeGlobal(m_bytes), "Failed to free adapter resources");
        m_bytes = nullptr;
    }

    // zero position and ownership
    m_position = 0;
    m_length = 0;
    m_end = nullptr;
}

void Adapter::seek(size_t _Amount, BIOSEEKDIR _Direction)
{
    switch(_Direction)
    {
    case E_BIO_SEEK_START:
    {
        if(_Amount >= m_length)
            return;

        m_position = _Amount;
        break;
    }
    case E_BIO_SEEK_CUR:
    {
        if(m_position + _Amount > m_length)
            return;

        m_position += _Amount;
        break;
    }
    case E_BIO_SEEK_END:
    {
        m_position = (m_length - _Amount);
        break;
    }
    }
}

s64 Adapter::getAvailableBytes() const
{
    return s64(length());
}

s64 Adapter::getPosition() const
{
    return m_position;
}

void Adapter::setPosition(s64 pos)
{
    m_position = pos;
}

BIORETURN Adapter::read(void* p, size_t len)
{
    if(!p)
        return E_BIO_ERR_NO_OUT_AVAIL;
    
    if(!len)
        return E_BIO_ERR_ARGUMENT;

    if(!m_bytes)
        return E_BIO_ERR_NO_IN_AVAIL;
    
    if(m_bytes + m_position >= m_end)
        return E_BIO_ERR_END_REACHED;

    size_t n = len;
    if((m_bytes + m_position + len) > m_end) {
        n = (size_t)(m_end - (m_bytes + m_position));
    }

    std::memcpy(p, m_bytes + m_position, n);
    m_position += n;

    return BIORETURN(n);
}

BIORETURN Adapter::write(const void* p, size_t len)
{
    if(!p)
        return E_BIO_ERR_NO_IN_AVAIL;

    if(!len)
        return E_BIO_ERR_NO_IN_AVAIL;

    if(!m_bytes)
        return E_BIO_ERR_NO_OUT_AVAIL;

    if((m_bytes + m_position + len) > m_end)
    {
        if(!m_isDynamic)
            return E_BIO_ERR_END_REACHED;

        if(!ensure(len))
            return E_BIO_ERR_BIOACCESS;
    }

    std::memcpy(m_bytes + m_position, p, len);
    if(m_length == m_position) 
        m_length += len;

    m_position += len;
    return BIORETURN(len);
}

bool Adapter::ensure(size_t len)
{
    if(!m_bytes)
        return false;

    size_t required = m_position + len;
    size_t current = size_t(m_end - m_bytes);
    if(required <= current)
        return true;    

    constexpr size_t align = kDefaultAdapterAlignment;
    size_t aligned = (required + (align - 1)) & ~(align - 1);
    if(aligned < required)
        return false;        
    
    u8* bytes = (u8*) HeapReallocGlobal(m_bytes, aligned, align);
    if(!bytes)
        return false;
    
    m_bytes = bytes;
    m_end = bytes + aligned;
    return true;
}
}