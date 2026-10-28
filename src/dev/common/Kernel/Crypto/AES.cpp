#include <Engine.Base/Crypto/AES.h>

#ifdef SvcCompilerMSVC
    #include <Windows.h>
    #include <bcrypt.h>
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-goto"

namespace rw
{
size_t AesCbcGetEncryptedSize(size_t size)
{
    return ((size / AesBlockSize) + 1) * AesBlockSize;
}

size_t AesEncrypt(AesKeySize keySize, AesCipherMode cipher, const void* key, const void* iv, const void* unencryptedData, size_t unencryptedSize, void* result, size_t resultSize, void* tag)
{
    ULONG bytesDone = 0;

    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;
    NTSTATUS status;

    status = BCryptOpenAlgorithmProvider(
        &hAlg,
        BCRYPT_AES_ALGORITHM,
        NULL,
        0
    );
    if (!BCRYPT_SUCCESS(status)) return status;

    {
        PUCHAR  pbChainingMode = NULL;
        ULONG   uChainingModeSize = 0;

        switch(cipher)
        {
            case AesCipherCBC: pbChainingMode = (PUCHAR)BCRYPT_CHAIN_MODE_CBC; uChainingModeSize = sizeof(BCRYPT_CHAIN_MODE_CBC); break;
            case AesCipherCFB: pbChainingMode = (PUCHAR)BCRYPT_CHAIN_MODE_CFB; uChainingModeSize = sizeof(BCRYPT_CHAIN_MODE_CFB); break;
            case AesCipherGCM: pbChainingMode = (PUCHAR)BCRYPT_CHAIN_MODE_GCM; uChainingModeSize = sizeof(BCRYPT_CHAIN_MODE_GCM); break;
        }

        __builtin_assert(pbChainingMode);

        status = BCryptSetProperty(
            hAlg,
            BCRYPT_CHAINING_MODE,
            pbChainingMode,
            uChainingModeSize,
            0
        );
        if (!BCRYPT_SUCCESS(status)) goto cleanup;
    }

    status = BCryptGenerateSymmetricKey(
        hAlg,
        &hKey,
        NULL,
        0,
        (PUCHAR)key,
        (ULONG)keySize,
        0
    );
    if (!BCRYPT_SUCCESS(status)) goto cleanup;

    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO info;

    u8 stackIv[AesIvSize]{0};

    if(cipher == AesCipherGCM)
    {
        BCRYPT_INIT_AUTH_MODE_INFO(info);

        info.pbNonce = (PUCHAR)iv;
        info.cbNonce = AesGcmIvSize;
        info.pbTag   = (PUCHAR)tag;
        info.cbTag   = tag ? AesTagSize : 0;
    }
    else
    {
        std::memcpy(stackIv, iv, AesIvSize);
    }

    status = BCryptEncrypt(
        hKey,
        (PUCHAR)unencryptedData,
        (ULONG)unencryptedSize,
        cipher == AesCipherGCM ? &info : NULL,
        cipher == AesCipherGCM ? NULL : stackIv,
        cipher == AesCipherGCM ? 0 : AesIvSize,
        (PUCHAR)result,
        (ULONG)resultSize,
        &bytesDone,
        cipher == AesCipherGCM ? 0 : BCRYPT_BLOCK_PADDING
    );

    if (!BCRYPT_SUCCESS(status))
    {
        bytesDone = 0;
        goto cleanup;
    }

cleanup:
    if (hKey) BCryptDestroyKey(hKey);
    if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
    return bytesDone;
}

size_t AesDecrypt(AesKeySize keySize, AesCipherMode cipher, const void* key, const void* iv, const void* encryptedData, size_t encryptedSize, void* result, size_t resultSize, const void* tag)
{
    ULONG bytesDone = 0;

    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;
    NTSTATUS status;

    status = BCryptOpenAlgorithmProvider(
        &hAlg,
        BCRYPT_AES_ALGORITHM,
        NULL,
        0
    );
    if (!BCRYPT_SUCCESS(status)) return status;

    {
        PUCHAR  pbChainingMode = NULL;
        ULONG   uChainingModeSize = 0;

        switch(cipher)
        {
            case AesCipherCBC: pbChainingMode = (PUCHAR)BCRYPT_CHAIN_MODE_CBC; uChainingModeSize = sizeof(BCRYPT_CHAIN_MODE_CBC); break;
            case AesCipherCFB: pbChainingMode = (PUCHAR)BCRYPT_CHAIN_MODE_CFB; uChainingModeSize = sizeof(BCRYPT_CHAIN_MODE_CFB); break;
            case AesCipherGCM: pbChainingMode = (PUCHAR)BCRYPT_CHAIN_MODE_GCM; uChainingModeSize = sizeof(BCRYPT_CHAIN_MODE_GCM); break;
        }

        __builtin_assert(pbChainingMode);

        status = BCryptSetProperty(
            hAlg,
            BCRYPT_CHAINING_MODE,
            pbChainingMode,
            uChainingModeSize,
            0
        );
        if (!BCRYPT_SUCCESS(status)) goto cleanup;
    }

    status = BCryptGenerateSymmetricKey(
        hAlg,
        &hKey,
        NULL,
        0,
        (PUCHAR)key,
        (ULONG)keySize,
        0
    );
    if (!BCRYPT_SUCCESS(status)) goto cleanup;

    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO info;

    u8 stackIv[AesIvSize] {0};

    if(cipher == AesCipherGCM)
    {
        BCRYPT_INIT_AUTH_MODE_INFO(info);

        info.pbNonce = (PUCHAR)iv;
        info.cbNonce = AesGcmIvSize;
        info.pbTag   = (PUCHAR)tag;
        info.cbTag   = tag ? AesTagSize : 0;
    }
    else
    {
        std::memcpy(stackIv, iv, AesIvSize);
    }

    status = BCryptDecrypt(
        hKey,
        (PUCHAR)encryptedData,
        (ULONG)encryptedSize,
        cipher == AesCipherGCM ? &info : NULL,
        cipher == AesCipherGCM ? NULL : (PUCHAR)stackIv,
        cipher == AesCipherGCM ? 0 : AesIvSize,
        (PUCHAR)result,
        (ULONG)resultSize,
        &bytesDone,
        cipher == AesCipherGCM ? 0 : BCRYPT_BLOCK_PADDING
    );

    if (!BCRYPT_SUCCESS(status))
    {
        bytesDone = 0;
        goto cleanup;
    }

cleanup:
    if (hKey) BCryptDestroyKey(hKey);
    if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
    return bytesDone;
}

#pragma clang diagnostic pop
}