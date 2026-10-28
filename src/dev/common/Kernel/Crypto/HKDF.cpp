#include <Engine.Base/Crypto/HKDF.h>
#include <Engine.Base/Crypto/SHAFunctions.h>

#define SHA_EXTRACT(f, var) \
SHA##f##Context var; \
SHA2::SHA##f##Initialize(var, salt, saltSize); \
SHA2::SHAUpdate(var, ikm, ikmSize)

#define SHA_EXPAND(f) \
SHA##f##Context sha2; \
SHA2::SHA##f##Initialize(sha2, prk, prkSize); \
SHA2::SHAUpdate(sha2, nextIn, availIn); \
SHA2::SHA##f##Finalize(sha2, lastHmac)

namespace rw::Crypto
{
bool HkdfExtract(HkdfMethod method, void* result, const void* salt, u32 saltSize, const void* ikm, u32 ikmSize)
{
    if(!ikm || !ikmSize)
    {
        return false;
    }
    if(!salt)
    {
        salt = (const void*)"";
        saltSize = 0;
    }

    switch(method)
    {
    case kAlgSHA1: 	 { SHA_EXTRACT(1, hmac); SHA2::SHA1Finalize(hmac, result); return true; }
    case kAlgSHA256: { SHA_EXTRACT(256, hmac); SHA2::SHA256Finalize(hmac, result); return true; }
    case kAlgSHA384: { SHA_EXTRACT(384, hmac); SHA2::SHA384Finalize(hmac, result); return true; }
    case kAlgSHA512: { SHA_EXTRACT(512, hmac); SHA2::SHA512Finalize(hmac, result); return true; }
    default: return false;
    }
}

bool HkdfExpand(HkdfMethod method, void* result, u32 resultSize, const void* prk, u32 prkSize, const void* info, u32 infoSize)
{
    if((!prk || !prkSize) || !resultSize)
    {
        return false;
    }
    if(!info)
    {
        info = (const void*)"";
        infoSize = 0;
    }

    uint8_t *lastHmac = (uint8_t *) _malloca(prkSize);
    std::memset(lastHmac, 0, prkSize);

    size_t chunkSize = prkSize + infoSize + 1;
    uint8_t* chunk = new uint8_t[chunkSize];
    std::memset(chunk, 0, chunkSize);

    size_t offset = 0;
    for(size_t n = 0; offset < resultSize; n++)
    {
        uint8_t* nextOut	= (uint8_t *)result + offset;
        uint8_t* nextIn		= n==0 ? chunk+prkSize : chunk;
        size_t availIn		= n==0 ? chunkSize-prkSize : chunkSize;
        size_t availOut		= resultSize - offset;

        //1. copy T(n-1)
        if(n>0)
        {
            memcpy_s( chunk, prkSize, lastHmac, prkSize );
        }
        //2. copy info (ALWAYS CHUNK+SIZE_OF_SHA_HASH)
        memcpy_s( chunk+prkSize, infoSize, info, infoSize );
        nextIn[availIn-1] = (char)(n+1);

        switch(method)
        {
            case kAlgSHA1: { SHA_EXPAND(1); break; }
            case kAlgSHA256: { SHA_EXPAND(256); break; }
            case kAlgSHA384: { SHA_EXPAND(384); break; }
            case kAlgSHA512: { SHA_EXPAND(512); break; }
            default: return false;
        }

        memcpy_s(nextOut, availOut, lastHmac, (availOut < prkSize ? availOut : prkSize));
        offset += prkSize;
    }

    _freea(lastHmac);
    return true;
}

uint8_t* HkdfDerive(HkdfMethod method, const void* ikm, u32 ikmSize, u32 outputSize, const void* info, u32 infoSize, const void* salt, u32 saltSize)
{
    uint8_t prkSize = uint8_t(-1);
    switch(method)
    {
        case kAlgSHA1: prkSize = SHA1Context::kHashSize; break;
        case kAlgSHA256: prkSize = SHA256Context::kHashSize; break;
        case kAlgSHA384: prkSize = SHA384Context::kHashSize; break;
        case kAlgSHA512: prkSize = SHA512Context::kHashSize; break;
    }

    if(prkSize == uint8_t(-1))
    {
        return nullptr;
    }

    void *prk = _malloca(prkSize);
    memset(prk,0,prkSize);
    
    uint8_t* output = new uint8_t[outputSize];
    memset(output,0,outputSize);

    bool ret;
    ret = HkdfExtract(method, prk, salt, saltSize, ikm, ikmSize);
    if(!ret)
    {
        _freea(prk);
        return nullptr;
    }
    ret = HkdfExpand(method, output, outputSize, prk, prkSize, info, infoSize);
    if(!ret)
    {
        _freea(prk);
        return nullptr;
    }		

    _freea(prk);
    return output;
}

bool HkdfDerive(HkdfMethod method, const void* ikm, u32 ikmSize, void* output, u32 outputSize, const void* info, u32 infoSize, const void* salt, u32 saltSize)
{
    uint8_t prkSize = uint8_t(-1);
    switch(method)
    {
        case kAlgSHA1: prkSize = SHA1Context::kHashSize; break;
        case kAlgSHA256: prkSize = SHA256Context::kHashSize; break;
        case kAlgSHA384: prkSize = SHA384Context::kHashSize; break;
        case kAlgSHA512: prkSize = SHA512Context::kHashSize; break;
    }

    if(prkSize == uint8_t(-1))
    {
        return NO;
    }

    void *prk = _malloca(prkSize);
    memset(prk,0,prkSize);
    
    memset(output,0,outputSize);

    bool ret;
    ret = HkdfExtract(method, prk, salt, saltSize, ikm, ikmSize);
    if(!ret)
    {
        _freea(prk);
        return NO;
    }
    ret = HkdfExpand(method, output, outputSize, prk, prkSize, info, infoSize);
    if(!ret)
    {
        _freea(prk);
        return NO;
    }		

    _freea(prk);
    return YES;
}
}