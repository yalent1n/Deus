#include <Engine.Base/Crypto/ECDH.h>

#ifdef SvcCompilerMSVC
    #include <Windows.h>
    #include <bcrypt.h>
#endif

#define _ECDH_ALG_OFFSET 0
#define _ECDH_PUB_OFFSET 8
#define _ECDH_PRI_OFFSET 16

namespace rw
{
bool EcdhInitialize(ECDHContext* pContext, bool generate_keys)
{
    if(!pContext)
        return false;

    BCRYPT_ALG_HANDLE& hAlg = *(BCRYPT_ALG_HANDLE*)&pContext->bytes[_ECDH_ALG_OFFSET];
    BCRYPT_KEY_HANDLE& hPublicKey = *(BCRYPT_KEY_HANDLE*)&pContext->bytes[_ECDH_PUB_OFFSET];
    BCRYPT_KEY_HANDLE& hPrivateKey = *(BCRYPT_KEY_HANDLE*)&pContext->bytes[_ECDH_PRI_OFFSET];
    
    NTSTATUS status;
    status = BCryptOpenAlgorithmProvider(
        &hAlg,
        BCRYPT_ECDH_P256_ALGORITHM,
        NULL,
        0
    );

    if (!BCRYPT_SUCCESS(status)) 
        return false;

    if(generate_keys)
    {
        status = BCryptGenerateKeyPair(hAlg, &hPublicKey, 256, 0);
        if (!BCRYPT_SUCCESS(status)) 
            return false;

        status = BCryptFinalizeKeyPair(hPublicKey, 0);
        if (!BCRYPT_SUCCESS(status)) 
            return false;
    }
    else
    {
        hPublicKey = NULL;
    }

    hPrivateKey = hPublicKey;
    return true;
}

bool EcdhDestroy(ECDHContext* pContext)
{
    if(!pContext)
        return false;

    NTSTATUS status;
    BCRYPT_ALG_HANDLE& hAlg = *(BCRYPT_ALG_HANDLE*)&pContext->bytes[_ECDH_ALG_OFFSET];
    BCRYPT_KEY_HANDLE& hPublicKey = *(BCRYPT_KEY_HANDLE*)&pContext->bytes[_ECDH_PUB_OFFSET];
    BCRYPT_KEY_HANDLE& hPrivateKey = *(BCRYPT_KEY_HANDLE*)&pContext->bytes[_ECDH_PRI_OFFSET];
    
    if(hPublicKey != NULL)
    {
        status = BCryptDestroyKey(hPublicKey);
        if (!BCRYPT_SUCCESS(status)) 
            return false;
    }

    if(hPrivateKey && hPrivateKey != hPublicKey)
    {
        status = BCryptDestroyKey(hPrivateKey);
        if (!BCRYPT_SUCCESS(status)) 
            return false;
    }

    if(hAlg != NULL)
    {
        status = BCryptCloseAlgorithmProvider(hAlg, 0);
        if (!BCRYPT_SUCCESS(status)) 
            return false;
    }

    hAlg = NULL;
    hPublicKey = NULL;
    hPrivateKey = NULL;
    return true;
}

bool EcdhImportPrivateKey(ECDHContext* pContext, EcdhPrivateKey* pPrivateKey)
{
    if(!pContext)
        return false;

    BCRYPT_ALG_HANDLE& hAlg = *(BCRYPT_ALG_HANDLE*)&pContext->bytes[_ECDH_ALG_OFFSET];
    BCRYPT_KEY_HANDLE& hPrivateKey = *(BCRYPT_KEY_HANDLE*)&pContext->bytes[_ECDH_PRI_OFFSET];

    NTSTATUS status = BCryptImportKeyPair(
        hAlg,
        NULL,
        BCRYPT_ECCPRIVATE_BLOB,
        &hPrivateKey,
        (PUCHAR)pPrivateKey,
        sizeof(EcdhPrivateKey),
        0
    );

    return BCRYPT_SUCCESS(status);
}

bool EcdhExportPrivateKey(ECDHContext* pContext, EcdhPrivateKey* pPrivateKey)
{
    if(!pContext)
        return false;

    BCRYPT_KEY_HANDLE& hKey = *(BCRYPT_KEY_HANDLE*)&pContext->bytes[_ECDH_PRI_OFFSET];

    ULONG nKeySize = 0;
    BCryptExportKey(
        hKey,
        NULL,
        BCRYPT_ECCPRIVATE_BLOB,
        NULL,
        0,
        &nKeySize,
        0
    );

    if(nKeySize != sizeof(EcdhPrivateKey))
        return false;

    BCryptExportKey(
        hKey,
        NULL,
        BCRYPT_ECCPRIVATE_BLOB,
        (PUCHAR)pPrivateKey,
        (ULONG)nKeySize,
        &nKeySize,
        0
    );

    return true;
}

bool EcdhImportPublicKey(ECDHContext* pContext, EcdhPublicKey* pPublicKey)
{
    BCRYPT_ALG_HANDLE& hAlg = *(BCRYPT_ALG_HANDLE*)&pContext->bytes[_ECDH_ALG_OFFSET];
    BCRYPT_KEY_HANDLE& hPublicKey = *(BCRYPT_KEY_HANDLE*)&pContext->bytes[_ECDH_PUB_OFFSET];

    NTSTATUS status = BCryptImportKeyPair(
        hAlg,
        NULL,
        BCRYPT_ECCPUBLIC_BLOB,
        &hPublicKey,
        (PUCHAR)pPublicKey,
        sizeof(EcdhPublicKey),
        0
    );

    return BCRYPT_SUCCESS(status);
}

bool EcdhExportPublicKey(ECDHContext* pContext, EcdhPublicKey* pPublicKey)
{
    BCRYPT_KEY_HANDLE& hKey = *(BCRYPT_KEY_HANDLE*)&pContext->bytes[_ECDH_PUB_OFFSET];

    ULONG nKeySize = 0;
    BCryptExportKey(
        hKey,
        NULL,
        BCRYPT_ECCPUBLIC_BLOB,
        NULL,
        0,
        &nKeySize,
        0
    );

    if(nKeySize != sizeof(EcdhPublicKey))
        return false;

    BCryptExportKey(
        hKey,
        NULL,
        BCRYPT_ECCPUBLIC_BLOB,
        (PUCHAR)pPublicKey,
        (ULONG)nKeySize,
        &nKeySize,
        0
    );

    return true;
}

bool EcdhComputeSharedSecret(ECDHContext* pContext, EcdhSharedSecret* pSecret)
{
    NTSTATUS status;
    BCRYPT_KEY_HANDLE& hPublicKey = *(BCRYPT_KEY_HANDLE*)&pContext->bytes[_ECDH_PUB_OFFSET];
    BCRYPT_KEY_HANDLE& hPrivateKey = *(BCRYPT_KEY_HANDLE*)&pContext->bytes[_ECDH_PRI_OFFSET];

    BCRYPT_SECRET_HANDLE hSecret = NULL;
    status = BCryptSecretAgreement(
        hPrivateKey,
        hPublicKey,
        &hSecret,
        0
    );

    if(!BCRYPT_SUCCESS(status))
        return false;

    ULONG nSecretSize = 0;
    BCryptDeriveKey(
        hSecret,
        BCRYPT_KDF_RAW_SECRET,
        NULL,
        NULL,
        0,
        &nSecretSize,
        0
    );

    if(nSecretSize != sizeof(EcdhSharedSecret))
        return false;

    status = BCryptDeriveKey(
        hSecret,
        BCRYPT_KDF_RAW_SECRET,
        NULL,
        (PUCHAR)pSecret,
        nSecretSize,
        &nSecretSize,
        0
    );

    return BCRYPT_SUCCESS(status);
}
}