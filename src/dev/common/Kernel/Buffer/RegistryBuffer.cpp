#include <Engine.Base/Buffer/RegistryBuffer.h>

namespace rw
{
RegistryBuffer::RegistryBuffer()
    : m_key(nullptr)
    , m_subKey(nullptr)
{}

RegistryBuffer::RegistryBuffer(RegistryKey* key, BIOACCESS access, const char* subKey)    
    : m_key(key)
    , m_subKey(subKey)
{
    m_access = access;
}

void RegistryBuffer::flush()
{
    if(m_key)
    {        
        m_key = nullptr;
        m_subKey = nullptr;
    }
}

BIORETURN RegistryBuffer::read(void* p, size_t maxlen)
{
    if(!(m_access & E_BIO_READ))
        return E_BIO_ERR_BIOACCESS;

    if(!m_key)
        return E_BIO_ERR_NO_IN_AVAIL;

    if(!maxlen)
        return E_BIO_ERR_ARGUMENT;

    if(!p)
        return E_BIO_ERR_NO_OUT_AVAIL;

    return m_key->Read(p, maxlen, m_subKey) ? E_BIO_SUCCESS : E_BIO_ERR_OS;    
}

BIORETURN RegistryBuffer::write(const void* p, size_t maxlen)
{
    if(!(m_access & E_BIO_WRITE))
        return E_BIO_ERR_BIOACCESS;

    if(!m_key)
        return E_BIO_ERR_NO_OUT_AVAIL;

    if(!maxlen)
        return E_BIO_ERR_ARGUMENT;

    if(!p)
        return E_BIO_ERR_NO_IN_AVAIL;

    return m_key->Write(p, maxlen, m_subKey) ? E_BIO_SUCCESS : E_BIO_ERR_OS;
}
}