#include <Engine.Core/Backends/RegistryFsBackend.h>
#include <Engine.Base/Buffer/RegistryBuffer.h>

namespace rw
{
registry_view::registry_view(RKCLASS rkc, const char* key)
{
    m_key.Create(rkc, key);
}
registry_view::~registry_view()
{
    // The destructor closes it automatically.
    //m_key.Close();
}

// const char* registry_view::directory()
// {
//     return m_key.GetKeyName();
// }

Buffer* registry_view::touch(const char* subkey, BIOACCESS caps)
{
    u16 b = (caps | E_BIO_FLUSH);
    return new RegistryBuffer(&m_key, BIOACCESS(b), subkey);
}

Buffer* registry_view::openFile(const char* subkey, bool check)
{
    u16 caps = E_BIO_READ;
    return touch(subkey, BIOACCESS(caps));
}

Buffer* registry_view::createFile(const char* subkey, bool append, bool read)
{
    u16 caps = E_BIO_WRITE | (read ? E_BIO_READ : 0) | (append ? E_BIO_APPEND : 0);
    return touch(subkey, BIOACCESS(caps));
}
}