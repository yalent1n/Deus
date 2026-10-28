#pragma once
#include "Bundle.h"
#include <Engine.Base/Crypto/AES.h>
#include <Engine.Base/Crypto/SHA.h>
#include <Engine.Base/Crypto/SHAFunctions.h>
#include <Engine.Base/HashMap.h>

namespace rw
{
inline constexpr u32 kCryptoKeyIdSize = 128/8;
inline constexpr u32 kCryptoKeySize = AesKeySize256;
inline constexpr u32 kCryptoKeySignatureSize = Crypto::SHA256Context::kHashSize;

inline constexpr u32 kCryptoDomainKeySize = 512/8;
inline constexpr u32 kCryptoProviderKeySize = 128/8;

#define TRUST_VAULT_LOCATION "trustvault.xml"

struct CryptoKeyId
{
    u8 data[kCryptoKeyIdSize];
};

struct CryptoKey
{
    u8 data[kCryptoKeySize];
};

struct CryptoKeySignature
{
    u8 data[kCryptoKeySignatureSize];
};

struct CryptoDomainKey
{
    u8 data[kCryptoDomainKeySize];
};

struct CryptoProviderKey
{
    u8 data[kCryptoProviderKeySize]; // shown as a GUID
};

struct CryptoKeyIdHash
{
    unsigned operator()(const CryptoKeyId& keyid) const
    {
        return DJB(keyid.data, kCryptoKeyIdSize);
    }
};

struct CryptoProviderKeyHash
{
    unsigned operator()(const CryptoProviderKey& key) const
    {
        return DJB(key.data, kCryptoProviderKeySize);
    }
};

struct CryptoDomainKeyHash
{
    unsigned operator()(const CryptoDomainKey& key) const
    {
        return DJB(key.data, kCryptoDomainKeySize);
    }
};

struct CryptoKeyIdLess
{
    bool operator()(const CryptoKeyId& lhs, const CryptoKeyId& rhs) const
    {
        return std::memcmp(lhs.data, rhs.data, kCryptoKeyIdSize) < 0;
    }
};

struct CryptoKeyIdEquals
{
    bool operator()(const CryptoKeyId& lhs, const CryptoKeyId& rhs) const
    {
        return std::memcmp(lhs.data, rhs.data, kCryptoKeyIdSize) == 0;
    }
};

/*
    *** Key Derivation ***

    Introduce: Domain key, Provider key, context info.
    
    - Domain key: A uniquely random 512-bit bytestring.
    - Provider key: A uniquely random 128-bit bytestring.
    - Context Info: A context string for different access -> csm:{tag}:{scope(pub/iam)}:{providerKey}
*/

/* [IAD] Identity Access Domain: Specific use-cases (like a user bucket, user token, etc.) */
#define E_DOM_IDENTITY 1

/* [GAD] Generalized Access Domain: Public use-cases (like a public bucket, key exchange, sign key, etc.) */
#define E_DOM_GENERALIZED 2

/* [PAD] Personal Access Domain: Private use-cases (like a connection, file, pipe, etc.) */
#define E_DOM_PERSONAL 3

struct CryptoParams
{
    CryptoDomainKey domainKey;
    CryptoProviderKey providerKey;
    eastl::string tag;
    int domain;
};

/*
    Generates a new key ID, key and checksum for storage encryption.
*/
FB_STORAGE_API bool CryptoProvider_newKey(const CryptoParams& params, CryptoKeyId& keyId, CryptoKey& key, CryptoKeySignature& checkSum);

/*
    Validates a key using the crypto parameters.
*/
FB_STORAGE_API bool CryptoProvider_validateKey(const CryptoDomainKey& domainKey, const CryptoProviderKey& providerKey, const CryptoKey& key, const CryptoKeySignature& checkSum);

interface IKeyManager
{
    struct KeyHandle { CryptoKey key; CryptoKeySignature sig; };
    typedef eastl::hash_map<CryptoKeyId, KeyHandle, CryptoKeyIdHash> Keys_t;
    
    nodefine const CryptoDomainKey* GetDomainKey() const = 0;
    nodefine const CryptoProviderKey* GetProviderKey() const = 0;

    nodefine bool FetchKeyInfo(const CryptoKeyId& keyId, CryptoKey* key, CryptoKeySignature* signature) const = 0;
    nodefine bool AddKeyInfo(const CryptoKeyId& keyId, const CryptoKey* key, const CryptoKeySignature* signature) = 0;
    
    nodefine bool GenerateKey(int domain, CryptoKeyId& keyId, CryptoKey* key, CryptoKeySignature* signature) = 0;
};

// TODO: somehow put everything under the "Deus Group Socials" namespace or in shorter term "dgsc"

class DgscKeyManager : public IKeyManager
{
public:
    FB_STORAGE_API DgscKeyManager();
    FB_STORAGE_API ~DgscKeyManager();

    const CryptoDomainKey* GetDomainKey() const
        { return &m_DomainKey; }

    const CryptoProviderKey* GetProviderKey() const
        { return &m_ProviderKey; }

    bool FetchKeyInfo(const CryptoKeyId& keyId, CryptoKey* key, CryptoKeySignature* signature) const override;
    bool AddKeyInfo(const CryptoKeyId& keyId, const CryptoKey* key, const CryptoKeySignature* signature) override;

    bool GenerateKey(int domain, CryptoKeyId& keyId, CryptoKey* key, CryptoKeySignature* signature) override;

private:
    CryptoDomainKey m_DomainKey;
    CryptoProviderKey m_ProviderKey;
    IKeyManager::Keys_t m_Keys;
};

FB_STORAGE_API IKeyManager* _DgscKeyManager();
}