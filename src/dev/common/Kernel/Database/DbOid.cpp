#include <Engine.Base/Database/DbObject.h>

#ifdef SvcCompilerMSVC
    #include <Windows.h>
    #include <bcrypt.h>
#endif

namespace rw
{
static u32 s_sessionId = 0;
static u32 s_serial = 0;

DbOid DbOid::c_zero;

DbOid::DbOid()
    : id()
{}

void DbOid::init()
{
    u32 data[2];
    BCryptGenRandom(nullptr, reinterpret_cast<PUCHAR>(data), 8, BCRYPT_USE_SYSTEM_PREFERRED_RNG);

    s_sessionId = data[0];
    s_serial = data[1];
}

// The generator swaps endian because we want the
// string representation to look like a contiguous
// byte array and because of this, when getting the serial
// from 'serial()' we swap endian again (for sorting).
void DbOid::generate()
{
    Timestamp time = Time::getSystemMsTime();
    
    id[0] = static_cast<u32>(time);
    id[1] = endianSwap(s_sessionId);
    id[2] = endianSwap(++s_serial);
}

void DbOid::toString(StringEditor& str) const
{
    tohexstr(this, sizeof(DbOid), str);
}

bool DbOid::parse(const char* str)
{
    size_t len = strlen(str);
    if(len != sizeof(DbOid) * 2)
        return false;

    fromhexstr(str, len, this, sizeof(DbOid));
    return true;
}

bool DbOid::parse(const char* str, size_t len)
{
    if(isnil(str))
        return false;
    
    if(len != sizeof(DbOid) * 2)
        return false;

    fromhexstr(str, len, this, sizeof(DbOid));
    return true;
}

u32 DbOid::session() const
{
    return id[1];
}

u32 DbOid::serial() const
{
    return endianSwap(id[2]);
}
}