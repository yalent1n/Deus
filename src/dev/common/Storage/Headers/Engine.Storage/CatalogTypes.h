#pragma once
#include "Bundle.h"
#include <Engine.Base/Crypto/Guid.h>

namespace rw
{
enum FluxPatchType : u8
{
    FluxPatchType_None,
    FluxPatchType_New,
    FluxPatchType_CompressedDelta,
    FluxPatchType_UncompressedDelta,
    FluxPatchType_Remove
};

template<int RootBits, int CatalogBits, int FileBits>
struct CasFileId
{
    static constexpr u32 LastRootId = _MASK(RootBits);
    static constexpr u32 LastCatalogId = _MASK(CatalogBits);
    static constexpr u32 LastFileId = _MASK(FileBits);

    CasFileId(u32 root, u32 catalog, u32 file)
    {
        this->root = root;
        this->catalog = catalog;
        this->file = file;
    }

    u32 root : RootBits;
    u32 catalog : CatalogBits;
    u32 file : FileBits;
};

struct CasStandardFileId : CasFileId<2, 6, 24>
{
    static constexpr u32 BaseRoot  = 0b00;
    static constexpr u32 HeadRoot  = 0b01;
    static constexpr u32 PatchRoot = 0b10;

    static constexpr u32 GlobalCatalog = 0b000000;

    CasStandardFileId(u32 root = BaseRoot, u32 catalog = GlobalCatalog, u32 file = 0)
        : CasFileId<2, 6, 24>(root, catalog, file)
    {}
};

typedef SHA256 ContentId;
typedef Sha256Str ContentIdStr;

struct CasCatalogHeader
{
    /*
    ** NOTE:
    ** If you change something in the structure of extents/catalogs, update these fields immediately.
    */
    FB_STORAGE_API static const Guid SchemaVersion;
    FB_STORAGE_API static const Guid EncryptedSchemaVersion;

    Guid schemaVersion;
    size_t totalBytesUsed;
    size_t totalBytesCleanup;
    u32 entryCount;
    u32 patchedEntryCount;
    u32 encryptedEntryCount;
};

struct CatEntry
{
    enum Flags : u32
    {
        Compressed = 0x01,
        Delta = 0x02,
        Erased = 0x04,
        Pending = 0x08
    };

    ContentId sha;
    u32 offset;
    u32 size;
    u32 originalSize;
    u32 flagsAndCluster;

    u32 cluster() const { return u32((flagsAndCluster & 0xFF000000) >> 24); }
    void setCluster(u8 cluster) { flagsAndCluster |= (u32(cluster) << 24); }

    bool isCompressed() const { return !!(flagsAndCluster & Compressed); }
    bool isDelta() const { return !!(flagsAndCluster & Delta); }
    bool isErased() const { return !!(flagsAndCluster & Erased); }
    bool isPending() const { return !!(flagsAndCluster & Pending); }

    CatEntry()
    {
        memset(this, 0, sizeof(CatEntry));
    }
};

struct CatPatchEntry
{
    ContentId sha;
    ContentId base;
    ContentId delta;
};

struct CatCryptoEntry
{
    CatEntry    cat;
    u8          keyId[16];
    u8          checkSum[32];
};
}