#include <Engine.Storage/IoTrustVault.h>
#include <Engine.Base/Buffer/Adapter.h>
#include <Engine.Base/Crypto/HKDF.h>
#include <Engine.Base/Crypto/Base64.h>
#include <Engine.Base/String/String.h>
#include <Engine.Base/Misc/Xml.h>
#include <Engine.Core/diag/ExecutionContext.h>

#ifdef SvcCompilerMSVC
    #include <Windows.h>
    #include <bcrypt.h>
#endif

namespace rw
{
inline constexpr Crypto::HkdfMethod CryptoProvider_method = Crypto::kAlgSHA256;
inline constexpr u32 CryptoProvider_methodSize = Crypto::SHA256Context::kHashSize;

bool CryptoProvider_newKey(const CryptoParams& params, CryptoKeyId& keyId, CryptoKey& key, CryptoKeySignature& checkSum)
{
    /* phase 1: extract PRK from provider key (HKDF salt) and domain key (HKDF input keying material). */
    u8 prk[CryptoProvider_methodSize];
    if(!Crypto::HkdfExtract(CryptoProvider_method, prk, params.providerKey.data, sizeof(params.providerKey), params.domainKey.data, sizeof(params.domainKey)))
        return false;

    const char* scope;
    switch(params.domain)
    {
        case E_DOM_IDENTITY:
            scope = "iam"; 
            break;
        case E_DOM_GENERALIZED:
            scope = "pub"; 
            break;
        case E_DOM_PERSONAL:
            scope = "pri"; 
            break;
        default:
            return false;
    }

    /* phase 2: create key ID (random bytes). */
    BCryptGenRandom(NULL, keyId.data, sizeof(keyId.data), BCRYPT_USE_SYSTEM_PREFERRED_RNG);

    /* phase 3: create context info from tag, scope, provider key. */
    // NOTE: tag is usually an username.
    FixedStringBuilder<256> contextInfo;
    contextInfo << "csm:" << params.tag.c_str() << ':' << scope << ':';
    tohexstr(params.providerKey.data, sizeof(params.providerKey), contextInfo);

    // phase 4: extract actual encryption key.
    u8 result[kCryptoKeySize];
    if(!Crypto::HkdfExpand(CryptoProvider_method, result, sizeof(result), prk, sizeof(prk), contextInfo.c_str(), contextInfo.size()))
        return false;

    std::memcpy(key.data, result, kCryptoKeySize);

    // phase 5: calculate key signature with PRK (validated by domain and provider)
    ((SHA256*)checkSum.data)->calculateHmac(prk, CryptoProvider_methodSize, key.data, kCryptoKeySize);
    return true;
}

bool CryptoProvider_validateKey(const CryptoDomainKey& domainKey, const CryptoProviderKey& providerKey, const CryptoKey& key, const CryptoKeySignature& checkSum)
{
    u8 prk[CryptoProvider_methodSize];
    if(!Crypto::HkdfExtract(CryptoProvider_method, prk, providerKey.data, sizeof(providerKey.data), domainKey.data, sizeof(domainKey.data)))
        return false;

    SHA256 checkSumSha;
    checkSumSha.calculateHmac(key.data, kCryptoKeySize, prk, CryptoProvider_methodSize);
    return memcmp(checkSumSha.hash, checkSum.data, kCryptoKeySignatureSize) == 0;
}

void iotrust_open(CryptoDomainKey& domainKey, CryptoProviderKey& providerKey, IKeyManager::Keys_t& keys)
{
    FsBackend* fs = ExecutionContext::getAppFileSystem();
    ScopedRef xmlBuf = fs->shareFile(TRUST_VAULT_LOCATION);
    if(!xmlBuf)
    {
        BCryptGenRandom(NULL, domainKey.data, sizeof(domainKey.data), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        BCryptGenRandom(NULL, providerKey.data, sizeof(providerKey.data), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        EDEBUG("IO trust: bootsrapped successfully (without vault).");
        return;
    }

    XmlDocument* doc = XML_Parse(xmlBuf);
    if(!doc)
    {
        EERROR("IO trust: failed to parse vault file.");
        return;
    }

    XmlDocumentNode* xml = doc->getNode();
    if(!equalsIgnoreCase("Vault", xml->getNodeName()))
    {
        EERROR("IO trust: vault is not a valid file.");
        return;
    }

    const char* domainKeyAttr = xml->getAttribute("DomainKey");
    if(isnil(domainKeyAttr))
    {
        EERROR("IO trust: vault misses DomainKey attribute.");
        return;
    }

    const char* providerKeyAttr = xml->getAttribute("ProviderKey");
    if(isnil(providerKeyAttr))
    {
        EERROR("IO trust: vault misses ProviderKey attribute.");
        return;
    }

    size_t domainKeyAttrLen = strlen(domainKeyAttr);
    if(base64_decodelength(domainKeyAttrLen) != sizeof(CryptoDomainKey))
        return;
    
    size_t providerKeyAttrLen = strlen(providerKeyAttr);
    if(base64_decodelength(providerKeyAttrLen) != sizeof(CryptoProviderKey))
        return;

    if(!base64_urldecode(domainKeyAttr, domainKeyAttrLen, &domainKey, sizeof(domainKey.data)))
    {
        EERROR("IO trust: failed to parse DomainKey");
        return;
    }
    
    if(!base64_urldecode(providerKeyAttr, providerKeyAttrLen, &providerKey, sizeof(providerKey.data)))
    {
        EERROR("IO trust: failed to parse ProviderKey");
        return;
    }

    XmlDocumentNode* keyNode = xml->getChild();
    while(keyNode)
    {
        if(!equalsIgnoreCase("Key", keyNode->getNodeName()))
            goto _continue;
        
        const char* keyIdAttr = keyNode->getAttribute("Identifier");
        if(isnil(keyIdAttr))
            goto _continue;

        const char* keySignatureAttr = keyNode->getAttribute("IntegrityLabel");
        if(isnil(keySignatureAttr))
            goto _continue;
        
        const char* keyText = keyNode->getText();
        if(isnil(keyText))
            goto _continue;
        
        size_t keyIdAttrLen = strlen(keyIdAttr);
        if(base64_decodelength(keyIdAttrLen) != sizeof(CryptoKeyId))
            goto _continue;

        size_t keySignatureAttrLen = strlen(keySignatureAttr);
        if(base64_decodelength(keySignatureAttrLen) != sizeof(CryptoKeySignature))
            goto _continue;

        size_t keyTextLen = strlen(keyText);
        if(base64_decodelength(keyTextLen) != sizeof(CryptoKey))
            goto _continue;
        
        CryptoKeyId keyId;
        if(!base64_urldecode(keyIdAttr, keyIdAttrLen, &keyId, sizeof(keyId)))
            goto _continue;

        CryptoKey key;
        if(!base64_urldecode(keyText, keyTextLen, &key, sizeof(key)))
            goto _continue;

        CryptoKeySignature keySignature;
        if(!base64_urldecode(keySignatureAttr, keySignatureAttrLen, &keySignature, sizeof(keySignature)))
            goto _continue;

        keys.insert(keyId, {key, keySignature});

_continue:
        keyNode = keyNode->getSibling();
    }

    EDEBUG("IO trust: bootsrapped successfully.");
}

void iotrust_close(CryptoDomainKey& domainKeyRef, CryptoProviderKey& providerKeyRef, const IKeyManager::Keys_t& keys)
{
    FsBackend* fs = ExecutionContext::getAppFileSystem();
    ScopedRef xmlBuf = fs->createFile(TRUST_VAULT_LOCATION);
    if(!xmlBuf)
    {
        EERROR("IO trust: failed to create vault file.");
        return;
    }

    XmlDocument doc;
    
    XmlDocumentNode* xml = doc.createElement("Vault");
    doc.appendChild(xml);

    StringBuilder<> domainKey;
    base64_urlencode(&domainKeyRef, sizeof(domainKeyRef.data), domainKey);
    xml->setAttribute("DomainKey", domainKey.detach());

    StringBuilder<> providerKey;
    base64_urlencode(&providerKeyRef, sizeof(providerKeyRef.data), providerKey);
    xml->setAttribute("ProviderKey", providerKey.detach());

    for(const auto& kvp : keys)
    {
        StringBuilder<> keyId;
        base64_urlencode(&kvp.first, sizeof(kvp.first), keyId);
        
        StringBuilder<> key;
        base64_urlencode(&kvp.second.key, sizeof(kvp.second.key), key);

        StringBuilder<> keySignature;
        base64_urlencode(&kvp.second.sig, sizeof(kvp.second.sig), keySignature);

        XmlDocumentNode* keyNode = doc.createElement("Key", key.detach());
        keyNode->setAttribute("Identifier", keyId.detach());
        keyNode->setAttribute("IntegrityLabel", keySignature.detach());
        xml->appendChild(keyNode);
    }

    SkeletonBuildParams params;
    params.pretty = true;
    params.pretty_text = false;

    XmlDocumentBuilder builder(xmlBuf);
    bool xmlBuildSuccess = builder.buildSkeleton(&doc, &params);
    if(!xmlBuildSuccess)
    {
        EERROR("IO trust: failed to build trust file.");
    }

    EDEBUG("IO trust: closed");
}

DgscKeyManager::DgscKeyManager()
{
    //iotrust_open(m_DomainKey, m_ProviderKey, m_Keys);
}

DgscKeyManager::~DgscKeyManager()
{
    //iotrust_close(m_DomainKey, m_ProviderKey, m_Keys);
}

bool DgscKeyManager::FetchKeyInfo(const CryptoKeyId& keyId, CryptoKey* key, CryptoKeySignature* signature) const
{
    if(auto keyNode = m_Keys.find(keyId); keyNode != m_Keys.end())
    {
        if(key)
            *key = keyNode->second.key;
        
        if(signature)
            *signature = keyNode->second.sig;

        return true;
    }

    return false;
}

bool DgscKeyManager::AddKeyInfo(const CryptoKeyId& keyId, const CryptoKey* key, const CryptoKeySignature* signature)
{
    auto& kvp = m_Keys[keyId];
    if(key)
    kvp.key = *key;
    
    if(signature)
    kvp.sig = *signature;
    
    return true;
}

bool DgscKeyManager::GenerateKey(int domain, CryptoKeyId& keyId, CryptoKey* key, CryptoKeySignature* signature)
{
    CryptoParams params;
    params.domainKey = m_DomainKey;
    params.providerKey = m_ProviderKey;
    params.domain = domain;
    params.tag = "Deus Group";

    CryptoKeyId tmp_keyId;
    CryptoKey tmp_key;
    CryptoKeySignature tmp_keySignature;
    if(!CryptoProvider_newKey(params, tmp_keyId, tmp_key, tmp_keySignature))
    {
        return false;
    }

    keyId = tmp_keyId;

    if(key)
        *key = tmp_key;

    if(signature)
        *signature = tmp_keySignature;

    return _DgscKeyManager()->AddKeyInfo(keyId, &tmp_key, &tmp_keySignature);
}

IKeyManager* _DgscKeyManager()
{
    // if(!g_ioServiceEnabled)
    //     return nullptr;

    static DgscKeyManager* m_vault;
    
    RUN_ONCE(m_vault);
    m_vault = new DgscKeyManager();

    RAN_ONCE = YES;
    return m_vault;
}
}