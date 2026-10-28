#pragma once
#include "Macros.h"
#include <new>
#include <cstring>

namespace rw
{
typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
typedef unsigned int uint;
typedef long long s64;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;

typedef u16 sysResult;

#if __wordsize == 64
typedef long long ssize_t;
typedef unsigned long long size_t;
#else
typedef int ssize_t;
typedef unsigned int size_t;
#endif

typedef s64 difference_type;
typedef s64 StreamSize;
typedef size_t word;

// ----
// NOTE: I didn't know where to put these functions, so I appended it to the most used header.
// ----

#define _HASH(s) hashQuickLowerCaseConstExpr(s)
#define _HASH64(s) hashQuickLowerCaseConstExpr64(s)
#define REFL_HASH_CONSTEXPR(s) hashQuickLowerCaseConstExpr(s)

inline u32 DJB(const void* data, size_t size)
{
    unsigned hash = 5381;

    while (size-- > 0)
        hash = ((hash << 5) + hash) + static_cast<const char*>(data)[size];

    return hash;
}

inline u64 hashQuick64(const char* data, u64 hash = 5381)
{
    if (!data)
        return 0;
    u64 c;

    while ((c = (*data++)))
        hash = hash * 33 ^ c;

    return hash;
}

inline u64 hashQuickLowerCase64(const char* data, u64 hash = 5381)
{
    if (!data)
        return 0;
    u64 c;

    while ((c = (*data++)))
        hash = (hash * 33) ^ ((c >= 'a' && c <= 'z') ? c + 32 : c);

    return hash;
}

inline constexpr u64 hashQuickConstExpr64(const char* data, u64 hash = 5381)
{
    if (!data)
        return 0;
    return !*data
        ? hash
        : hashQuickConstExpr64(data + 1, hash * 33 ^ *data);
}

inline constexpr u64 hashQuickLowerCaseConstExpr64(const char* data, u64 hash = 5381)
{
    if (!data)
        return 0;
    return !*data
        ? hash
        : hashQuickConstExpr64(data + 1, hash * 33 ^ ((*data >= 'a' && *data <= 'z') ? *data + 32 : *data));
}

inline u32 hashQuick(const char* data, u32 hash = 5381)
{
    if (!data)
        return 0;
    u32 c;
    while ((c = (*data++)))
        hash = hash * 33 ^ c;

    return hash;
}
inline u32 hashQuickLowerCase(const char* data, u32 hash = 5381)
{
    if (!data)
        return 0;
    u32 c;
    while ((c = (*data++)))
        hash = hash * 33 ^ ((c >= 'a' && c <= 'z') ? c + 32 : c);

    return hash;
}

inline constexpr u32 hashQuickConstExpr(const char* data, u32 hash = 5381)
{
    if(!data)
        return 0;
    return !*data
        ? hash
        : hashQuickConstExpr(data + 1, hash * 33 ^ *data);
}
inline constexpr u32 hashQuickLowerCaseConstExpr(const char* data, u32 hash = 5381)
{
    if(!data)
        return 0;
    return !*data
        ? hash
        : hashQuickLowerCaseConstExpr(data + 1, hash * 33 ^ ((*data >= 'a' && *data <= 'z') ? *data + 32 : *data));
}
}