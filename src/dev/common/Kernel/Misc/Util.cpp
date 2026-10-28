#include <Engine.Base/Misc/Util.h>

namespace rw
{
u32 decodeSize(const void* _Ptr)
{
    const u8* p = (const u8*) _Ptr;
    
    u32 sh = 0;
    while (true)
    {
        if (sh == 35)
            break;

        u32 n = *p++;

        if (n >> 7 == 0)
            break;

        sh += 7;
    }

    return u32(p - (const u8*)_Ptr);
}

u32 decodeVarUint(const void* _Ptr)
{
    const u8* p = (const u8*) _Ptr;
    u32 ret = 0;
    u32 sh = 0;
    while (true)
    {
        if (sh == 35)
            break;

        u32 n = *p++;
        ret |= (n & 0x7F) << sh;

        if (n >> 7 == 0)
            break;

        sh += 7;
    }

    return ret;
}

u64 decodeVarUint64(const void* _Ptr)
{
    const u8* p = (const u8*) _Ptr;
    u64 ret = 0;
    u32 sh = 0;
    while (true)
    {
        if (sh == 35)
            break;

        u64 n = *p++;
        ret |= (n & 0x7F) << sh;

        if (n >> 7 == 0)
            break;

        sh += 7;
    }

    return ret;
}

s32 decodeVarInt(const void* _Ptr)
{
    u32 n = decodeVarUint(_Ptr);
    return (n >> 1) ^ -(s32)(n & 1);
}

s64 decodeVarInt64(const void* _Ptr)
{
    u64 n = decodeVarUint64(_Ptr);
    return (n >> 1) ^ -(s32)(n & 1);
}
}