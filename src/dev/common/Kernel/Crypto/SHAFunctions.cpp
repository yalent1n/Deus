#include <Engine.Base/Crypto/SHAFunctions.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <bcrypt.h>

namespace rw::Crypto
{
namespace detail
{
	CRESULT sha_init(SHAContext& ctx, LPCWSTR pwszAlgoId, const void* key, size_t keySize)
	{
		NTSTATUS			Status;
		
		if(NULL == pwszAlgoId)
		{
			return kCryptResult_ParameterInvalid;
		}

		Status = BCryptOpenAlgorithmProvider( &ctx.__alg, pwszAlgoId, NULL, key ? BCRYPT_ALG_HANDLE_HMAC_FLAG : 0 );
		if(!BCRYPT_SUCCESS( Status ))
		{
			return kCryptResult_Failed;
		}

		Status = BCryptCreateHash( ctx.__alg, &ctx.__hsh, NULL, 0, PUCHAR(key), ULONG(keySize), 0 );
		if(!BCRYPT_SUCCESS( Status ))
		{
			BCryptCloseAlgorithmProvider( ctx.__alg, 0 );
			ctx.__alg = NULL;
			return kCryptResult_Failed;
		}

		return kCryptResult_Succeeded;
	}

	CRESULT sha_finalize(SHAContext& ctx, PBYTE pbOutput)
	{
		ULONG _dummy;
		NTSTATUS Status;
		DWORD dwHashLen;

		Status = BCryptGetProperty( ctx.__alg, BCRYPT_HASH_LENGTH, (PBYTE)&dwHashLen, sizeof( dwHashLen ), &_dummy, 0 );
		if(!BCRYPT_SUCCESS( Status ))
		{
			BCryptDestroyHash( ctx.__hsh );
			ctx.__hsh = NULL;
			BCryptCloseAlgorithmProvider( ctx.__alg, 0 );
			ctx.__alg = NULL;
			return kCryptResult_Failed;
		}

		Status = BCryptFinishHash( ctx.__hsh, pbOutput, dwHashLen, 0 );
		if(!BCRYPT_SUCCESS( Status ))
		{
			BCryptDestroyHash( ctx.__hsh );
			ctx.__hsh = NULL;
			BCryptCloseAlgorithmProvider( ctx.__alg, 0 );
			ctx.__alg = NULL;
			return kCryptResult_Failed;
		}

		BCryptDestroyHash( ctx.__hsh );
		ctx.__hsh = NULL;
		BCryptCloseAlgorithmProvider( ctx.__alg, 0 );
		ctx.__alg = NULL;
		return kCryptResult_Succeeded;
	}
}
	
namespace SHA2
{
CRESULT SHA1Initialize(SHA1Context& context, const void* key, size_t key_size) {
    return detail::sha_init(context, BCRYPT_SHA1_ALGORITHM, key, key_size);
}
CRESULT SHA256Initialize(SHA256Context& context, const void* key, size_t key_size) {
    return detail::sha_init(context, BCRYPT_SHA256_ALGORITHM, key, key_size);
}
CRESULT SHA384Initialize(SHA384Context& context, const void* key, size_t key_size) {
    return detail::sha_init(context, BCRYPT_SHA384_ALGORITHM, key, key_size);
}
CRESULT SHA512Initialize(SHA512Context& context, const void* key, size_t key_size) {
    return detail::sha_init(context, BCRYPT_SHA512_ALGORITHM, key, key_size);
}

CRESULT SHAUpdate(SHAContext& context, const void* data, size_t data_size) { 
    if(!data || !data_size)
    {
        return kCryptResult_ParameterInvalid;
    }

    NTSTATUS Status = BCryptHashData( context.__hsh, (PBYTE)data, ULONG(data_size), 0 );
    if(!BCRYPT_SUCCESS( Status ))
    {
        BCryptDestroyHash( context.__hsh );
        context.__hsh = NULL;
        BCryptCloseAlgorithmProvider( context.__alg, 0 );
        context.__alg = NULL;
        return kCryptResult_Failed;
    }

    return kCryptResult_Succeeded;
}

CRESULT SHA1Finalize(SHA1Context& context, void* output) {
    return detail::sha_finalize(context, (PBYTE)output);
}
CRESULT SHA256Finalize(SHA256Context& context, void* output) {
    return detail::sha_finalize(context, (PBYTE)output);
}
CRESULT SHA384Finalize(SHA384Context& context, void* output) {
    return detail::sha_finalize(context, (PBYTE)output);
}
CRESULT SHA512Finalize(SHA512Context& context, void* output) {
    return detail::sha_finalize(context, (PBYTE)output);
}
}
}