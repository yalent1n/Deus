#pragma once
#include "Buffer.h"
#include <Engine.Base/RegistryKey.h>

namespace rw
{
class RegistryBuffer : public Buffer
{
public:
    FB_BASE_API RegistryBuffer();
    FB_BASE_API RegistryBuffer(RegistryKey*, BIOACCESS, const char* = NIL);

    FB_BASE_API void flush() override;
    FB_BASE_API BIORETURN read(void*, size_t) override;
    FB_BASE_API BIORETURN write(const void*, size_t) override;

private:
    RegistryKey*    m_key;
    const char*     m_subKey;
};
}