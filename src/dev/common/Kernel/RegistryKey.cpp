#include <Engine.Base/RegistryKey.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace rw
{
RegistryKey::RegistryKey()
    : mHandle(nullptr)
    , mKeyName(nullptr)
{}

RegistryKey::RegistryKey(RKCLASS rkc, const char* key)
    : mHandle(nullptr)
    , mKeyName(nullptr)
{
    (void) Open(rkc, key);
}

RegistryKey::~RegistryKey()
{
    (void) Close();
}

const char* RegistryKey::GetKeyName() const
{
    return mKeyName;
}

bool RegistryKey::Open(RKCLASS rkc, const char* key)
{    
    if(mHandle)
        return true;

    LONG Res = RegOpenKeyA(HKEY(rkc), key, PHKEY(&mHandle));
    if(Res != ERROR_SUCCESS)
    {
        mHandle = nullptr;
        return false;
    }
    
    mKeyName = key;
    return true;
}

bool RegistryKey::Create(RKCLASS rkc, const char* key)
{    
    if(mHandle)
        return true;    

    LONG Res = RegCreateKeyA(HKEY(rkc), key, PHKEY(&mHandle));
    if(Res != ERROR_SUCCESS)
    {
        mHandle = nullptr;
        return false;
    }
    
    mKeyName = key;
    return true;
}

bool RegistryKey::Close()
{
    if(mHandle)
    {        
        LSTATUS Stat = RegCloseKey(HKEY(mHandle));
        if(Stat != ERROR_SUCCESS)
        {
            return false;
        }
        
        mHandle = nullptr;
    }

    return true;
}

bool RegistryKey::Read(void* p, size_t len, const char* subkey) const
{
    ULONG cbData = ULONG(len);
    LSTATUS Stat = RegQueryValueExA(HKEY(mHandle), subkey, NIL, NIL, LPBYTE(p), &cbData);
    return Stat == ERROR_SUCCESS;
}

bool RegistryKey::Write(const void* p, size_t len, const char* subkey, RKTYPE rkt) const
{
    DWORD cbData = DWORD(len);
    LSTATUS Stat = RegSetValueExA(HKEY(mHandle), subkey, NIL, DWORD(rkt), (const BYTE*)(p), cbData);
    return Stat == ERROR_SUCCESS;    
}
}