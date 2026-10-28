#pragma once
#include "../FsBackend.h"
#include <Engine.Base/RegistryKey.h>

namespace rw
{
class registry_view : public FsBackend
{
public:
    CORE_API registry_view();
    CORE_API registry_view(RKCLASS, const char*);
    CORE_API ~registry_view();
    
    CORE_API Buffer* touch(const char*, BIOACCESS = E_BIO_ALWAYS_CREATE) override;
    CORE_API Buffer* openFile(const char*, bool check = true) override;
    CORE_API Buffer* createFile(const char*, bool append = true, bool read = true) override;

private:
    RegistryKey m_key;
};
}