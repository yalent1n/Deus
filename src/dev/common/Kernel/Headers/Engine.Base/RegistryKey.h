#pragma once
#include "Bundle.h"

namespace rw
{
enum RKCLASS : u32
{
    E_RC_CLASSES_ROOT        = 0x80000000,
    E_RC_CURRENT_CONFIG      = 0x80000005,
    E_RC_CURRENT_USER        = 0x80000001,
    E_RC_LOCAL_MACHINE       = 0x80000002,
    E_RC_USERS               = 0x80000003
};

enum RKTYPE : u32
{
    E_RT_NONE,
    E_RT_SZ,
    E_RT_EXPAND_SZ,
    E_RT_BINARY,
    E_RT_DWORD,
    E_RT_DWORD_LITTLE_ENDIAN,
    E_RT_DWORD_BIG_ENDIAN,
    E_RT_LINK,
    E_RT_MULTI_SZ,
    E_RT_RESOURCE_LIST,
    E_RT_FULL_RESOURCE_DESCRIPTOR,
    E_RT_RESOURCE_REQUIREMENTS_LIST,
    E_RT_QWORD,
    E_RT_QWORD_LITTLE_ENDIAN = E_RT_QWORD,
};

typedef void* KeyHandle;

class RegistryKey
{
public:
    FB_BASE_API RegistryKey();
    FB_BASE_API RegistryKey(RKCLASS rkc, const char* key); // opens a key, by default
    FB_BASE_API ~RegistryKey();

    FB_BASE_API const char* GetKeyName() const;

    // ----
    // I/O Functions

    FB_BASE_API bool Open(RKCLASS rkc, const char* key);
    FB_BASE_API bool Create(RKCLASS rkc, const char* key);
    FB_BASE_API bool Close();
    FB_BASE_API bool Read(void*, size_t, const char* subkey = NIL) const;
    FB_BASE_API bool Write(const void*, size_t, const char* subkey = NIL, RKTYPE rkt = E_RT_BINARY) const; // default type is REG_BINARY

private:
    KeyHandle mHandle;
    const char* mKeyName;
};
}