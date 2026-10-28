#include <Engine.Base/Crypto/ECC.h>

#ifdef SvcCompilerMSVC
    #include <Windows.h>
    #include <bcrypt.h>
#endif

bool ecc_p256_new(ECC_key& o) {
    NTSTATUS Status = BCryptGenRandom(NULL, (PUCHAR)&o, (ULONG)sizeof(ECC_key), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return BCRYPT_SUCCESS(Status);
}

