#pragma once
#include "../Bundle.h"

namespace rw
{
inline constexpr size_t AesBlockBitSize     = 128;
inline constexpr size_t AesBlockSize        = AesBlockBitSize / 8;

inline constexpr size_t AesIvBitSize        = 128;
inline constexpr size_t AesIvSize           = AesIvBitSize / 8;

inline constexpr size_t AesGcmIvBitSize     = 96;
inline constexpr size_t AesGcmIvSize        = AesGcmIvBitSize / 8;

inline constexpr size_t AesTagBitSize       = 128;
inline constexpr size_t AesTagSize          = AesTagBitSize / 8;

enum AesKeySize : u8
{
    AesKeySize128 = 128 / 8,
    AesKeySize192 = 192 / 8,
    AesKeySize256 = 256 / 8
};

enum AesCipherMode : u8
{
    AesCipherCBC,
    AesCipherCFB,
    AesCipherGCM, // 12 byte IV
};

FB_BASE_API size_t AesCbcGetEncryptedSize(size_t size);

FB_BASE_API size_t AesEncrypt(
    AesKeySize keySize, 
    AesCipherMode cipher, 
    const void* key, 
    const void* iv, 
    const void* unencryptedData, 
    size_t unencryptedSize,
    void* result,
    size_t resultSize, 
    /*optional*/ void* tag = NIL
);

FB_BASE_API size_t AesDecrypt(
    AesKeySize keySize, 
    AesCipherMode cipher, 
    const void* key, 
    const void* iv, 
    const void* encryptedData, 
    size_t encryptedSize,
    void* result,
    size_t resultSize, 
    /*optional*/ const void* tag = NIL);
}