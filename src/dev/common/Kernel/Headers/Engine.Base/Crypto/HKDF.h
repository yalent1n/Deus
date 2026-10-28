#pragma once
#include "../Bundle.h"

namespace rw::Crypto
{
enum HkdfMethod
{
    kAlgSHA1,
    kAlgSHA256,
    kAlgSHA384,
    kAlgSHA512
};

FB_BASE_API bool HkdfExtract(
    HkdfMethod, 
    void* result /*prk*/, 
    const void* salt, 
    u32 saltSize, 
    const void* ikm, 
    u32 ikmSize
);

FB_BASE_API bool HkdfExpand(
    HkdfMethod, 
    void* result /*okm*/, 
    u32 resultSize /*output length*/, 
    const void* prk, 
    u32 prkSize, 
    const void* info, 
    u32 infoSize
);

FB_BASE_API uint8_t* HkdfDerive(
    HkdfMethod,
    const void* ikm,
    u32 ikmSize,
    u32 outputSize,
    const void* info,
    u32 infoSize,
    const void* salt,
    u32 saltSize
);

FB_BASE_API bool HkdfDerive(
    HkdfMethod,
    const void* ikm,
    u32 ikmSize,
    void* output,
    u32 outputSize,
    const void* info,
    u32 infoSize,
    const void* salt,
    u32 saltSize
);
}