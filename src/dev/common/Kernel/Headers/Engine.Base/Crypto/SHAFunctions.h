#pragma once
#include "../Bundle.h"
#include "SHAConstExpr.h"

namespace rw::Crypto
{
enum CRESULT
{
    kCryptResult_Succeeded,
    kCryptResult_Failed,
    kCryptResult_OutOfRange,
    kCryptResult_NoMemory,
    kCryptResult_ParameterInvalid
};

struct SHAContext
{
	void* __alg; // algorithm provider
	void* __hsh; // hash handle
};
struct SHA1Context : SHAContext
{
    static constexpr unsigned kSizeBits = 160;
    static constexpr unsigned kHashSize = kSizeBits / 8;
};
struct SHA256Context : SHAContext
{
    static constexpr unsigned kSizeBits = 256;
    static constexpr unsigned kHashSize = kSizeBits / 8;
};
struct SHA384Context : SHAContext
{
    static constexpr unsigned kSizeBits = 384;
    static constexpr unsigned kHashSize = kSizeBits / 8;
};
struct SHA512Context : SHAContext
{
    static constexpr unsigned kSizeBits = 512;
    static constexpr unsigned kHashSize = kSizeBits / 8;
};

namespace SHA2
{
	// if key is non-null, then it's a MAC hash
	LIB_LINKAGE EXTERN_C CRESULT SHA1Initialize(SHA1Context&, const void* _Key, size_t _KeySize);
	LIB_LINKAGE EXTERN_C CRESULT SHA256Initialize(SHA256Context&, const void* _Key, size_t _KeySize);
	LIB_LINKAGE EXTERN_C CRESULT SHA384Initialize(SHA384Context&, const void* _Key, size_t _KeySize);
	LIB_LINKAGE EXTERN_C CRESULT SHA512Initialize(SHA512Context&, const void* _Key, size_t _KeySize);
	LIB_LINKAGE EXTERN_C CRESULT SHAUpdate(SHAContext&, const void*, size_t);
	LIB_LINKAGE EXTERN_C CRESULT SHA1Finalize(SHA1Context&, void* _Out);
	LIB_LINKAGE EXTERN_C CRESULT SHA256Finalize(SHA256Context&, void* _Out);
	LIB_LINKAGE EXTERN_C CRESULT SHA384Finalize(SHA384Context&, void* _Out);
	LIB_LINKAGE EXTERN_C CRESULT SHA512Finalize(SHA512Context&, void* _Out);
}
}

constexpr uint32_t EA_SHA1_32(const char* str);
constexpr uint64_t EA_SHA1_64(const char* str);
constexpr uint32_t EA_SHA256_32(const char* str)
{
	return Detail::SHA256_ConstExpr_32(str);
}
constexpr uint64_t EA_SHA256_64(const char* str)
{
	return Detail::SHA256_ConstExpr_64(str);
}
constexpr uint32_t EA_SHA384_32(const char* str);
constexpr uint64_t EA_SHA384_64(const char* str);
constexpr uint32_t EA_SHA512_32(const char* str);
constexpr uint64_t EA_SHA512_64(const char* str);