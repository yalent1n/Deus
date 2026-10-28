#pragma once
#include "../Bundle.h"

namespace rw
{
inline constexpr size_t ECDH_COORD_LENGTH = 32;

struct ECDHContext
{
    u8 bytes[24];
};

struct EcdhKey
{
    u32 _m; /*magic*/
    u32 _cl; /*coord length*/
    u8 X[ECDH_COORD_LENGTH];
    u8 Y[ECDH_COORD_LENGTH];
};

typedef EcdhKey EcdhPublicKey;

struct EcdhPrivateKey : EcdhKey
{
    u8 d[ECDH_COORD_LENGTH];
};

struct EcdhSharedSecret
{
    u8 bytes[ECDH_COORD_LENGTH];
};

// Initializes the ECDH context.
FB_BASE_API bool EcdhInitialize(ECDHContext*, bool = true/*generate_keys*/);

// Destroys the ECDH context.
FB_BASE_API bool EcdhDestroy(ECDHContext*);

// Imports the private key as blob.
FB_BASE_API bool EcdhImportPrivateKey(ECDHContext*, EcdhPrivateKey*);

// Exports the public key as blob and later the coords can be used separately.
FB_BASE_API bool EcdhExportPrivateKey(ECDHContext*, EcdhPrivateKey*);

// Imports the public key as blob.
FB_BASE_API bool EcdhImportPublicKey(ECDHContext*, EcdhPublicKey*);

// Exports the private key as blob and later the coords can be used separately.
FB_BASE_API bool EcdhExportPublicKey(ECDHContext*, EcdhPublicKey*);

// Exports the agreed secret without key derivation.
FB_BASE_API bool EcdhComputeSharedSecret(ECDHContext*, EcdhSharedSecret*);
}