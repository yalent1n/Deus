#pragma once
#include "BuildLayer.h"
#include <Engine.Base/HashMap.h>
#include <Engine.Base/FixedList.h>
#include <Engine.Base/Database/DbObjectBuilder.h>

namespace rw
{
// Note: An operation log of changes between deployed build images.
class BuildImageOpLog;

// Note: A build image containing the roots.
class BuildImage;

// Note: A build layout containing the information about install chunks and superbundles.
class BuildImageLayout;

// Note: A build manifest.
class BuildImageInstallManifest;

// Note: A container which groups together install chunks.
class BuildImageInstallGroup;

// Note: An installation unit.
class BuildImageInstallChunk;

// Note: An installation unit paired with the root it's in.
class BuildImageInstallChunkBuildLayer;

// Note: A superbundle.
class BuildImageSuperbundle;

// Note: A superbundle paired with the install chunk(s) it's in.
class BuildImageSuperbundleInstallChunk;

/*
    Note: The declaration of build image types are in reverse order for easier understanding of a database content layout.

    Layout Definition:
        Layout {
            Superbundles: []Object { name }
            Install Manifest : Object {
                Install chunks: []Object { name, installBundle, superbundles, (splitSuperbundles) }
            }
        }
*/

enum class SplitSuperbundleType : u8
{
    None,
    Deployed
};

struct CctorBackwards {};

template<class T> 
class BuildImageRangeIterator
{
public:
    template<class T2>
    BuildImageRangeIterator(Vec<T2>& range)
        : pPos(range.begin())
        , pPosStop(range.end())
    {}
    
    template<class T2>
    BuildImageRangeIterator(CctorBackwards, Vec<T2>& range)
        : pPos(range.end() - 1)
        , pPosStop(range.begin())
    {}

    T* operator->()
        { return pPos; }

    const T* operator->() const
        { return pPos; }
    
    T& operator*()
        { return *pPos; }
    
    const T& operator*() const
        { return *pPos; }
    
    operator T*() const
        { return pPos; }

    operator bool() const
        { return pPos < pPosStop; }

    BuildImageRangeIterator& operator++()
    {
        ++pPos;
        return *this;
    }
    
    BuildImageRangeIterator operator++(int)
    {
        BuildImageRangeIterator it = *this;
        ++it.pPos;
        return it;
    }
    
    BuildImageRangeIterator& operator--()
    {
        --pPos;
        return *this;
    }

    BuildImageRangeIterator operator--(int)
    {
        BuildImageRangeIterator it = *this;
        --it.pPos;
        return it;
    }

private:
    T* pPos;
    T* pPosStop;
};

/*
    An operation log is a container for log entries which has information about the build changes.
    In the layout's "headOp" and "baseOp" means exactly that. The last log serial number (LSN)
    of the current build and the previous build. -- and you could do something like headOp - baseOp = number of changes.

    Only used at deployment.
*/
class BuildImageOpLog
{
public:
    BuildImageOpLog(BuildImageLayout& layout)
        : m_layout(layout)
    {}

    const BuildImageLayout& getLayout() const { return m_layout; }
    BuildImageLayout& getLayout() { return m_layout; }

private:
    BuildImageLayout& m_layout;
};

/*
    Responsible for the bundles which are the loadable packages of the database.
    Superbundles can exist in multiple install chunks.
    By default superbundles has 1 assocciated install chunk,
    But if the superbundle has different purposes in other
    install chunks, then it can subchild to the ic.
*/
class BuildImageSuperbundle
{
public:
    typedef Vec<ScopedRef<BuildImageSuperbundleInstallChunk>> SuperbundleInstallChunks_t;

public:
    BuildImageSuperbundle(BuildImageLayout* layout = NIL, const char* name = NIL, u32 index = 0)
        : m_layout(*layout)
        , m_name(name)
        , m_index(index)
    {}

    virtual void parse(const DbObject& superbundleSpec);

    FB_STORAGE_API const char* getName() const;
    FB_STORAGE_API u32 getIndex() const;
    FB_STORAGE_API u32 getHash() const;

    FB_STORAGE_API const BuildImageLayout& getLayout() const { return m_layout; }
    template<class T> const T& getLayout() const { return m_layout; }
    FB_STORAGE_API BuildImageLayout& getLayout() { return m_layout; }
    template<class T> T& getLayout() { return m_layout; }

    FB_STORAGE_API const SuperbundleInstallChunks_t& getInstallChunks() const { return m_superbundleInstallChunks; }
    FB_STORAGE_API SuperbundleInstallChunks_t& getInstallChunks() { return m_superbundleInstallChunks; }

    template<class T>
    const BuildImageRangeIterator<T> getInstallChunks() const
        { return m_superbundleInstallChunks; }
    
    template<class T>
    BuildImageRangeIterator<T> getInstallChunks()
        { return m_superbundleInstallChunks; }

    FB_STORAGE_API BuildImageSuperbundleInstallChunk& createSuperbundleInstallChunk(SplitSuperbundleType split, BuildImageInstallChunk& ic);
private:
    BuildImageLayout& m_layout;
    const char* m_name;
    u32 m_index;
    SuperbundleInstallChunks_t m_superbundleInstallChunks;
};

/*
    Responsible for storing an sb-ic pair in the layout.
*/
class BuildImageSuperbundleInstallChunk
{
    friend class BuildImageSuperbundle;

public:
    // could be extended for more
    static constexpr u32 c_maxInstallChunks = 1;
    typedef fixed_vector<BuildImageInstallChunk*, c_maxInstallChunks> InstallChunks_t;

public:
    BuildImageSuperbundleInstallChunk(BuildImageSuperbundle* superbundle = NIL, SplitSuperbundleType splitType = SplitSuperbundleType::None)
        : m_superbundle(*superbundle)
        , m_splitType(splitType)
    {}

    SplitSuperbundleType getSplitType() const { return m_splitType; }

    const BuildImageSuperbundle& getSuperbundle() const { return m_superbundle; }
    template<class T> const T& getSuperbundle()const { return m_superbundle; }
    BuildImageSuperbundle& getSuperbundle() { return m_superbundle; }
    template<class T> T& getSuperbundle() { return m_superbundle; }

    const InstallChunks_t& getInstallChunks() const { return m_installChunks; }
    InstallChunks_t& getInstallChunks() { return m_installChunks; }

    template<class T>
    const BuildImageRangeIterator<T> getInstallChunks() const
        { return m_installChunks; }
    
    template<class T>
    BuildImageRangeIterator<T> getInstallChunks()
        { return m_installChunks; }

private:
    BuildImageSuperbundle& m_superbundle;
    InstallChunks_t m_installChunks;
    SplitSuperbundleType m_splitType;
};

/*
    Reponsible for an entry in the install chunk, which defines the root's instance (an can be used as a ic-root pair).
*/
class BuildImageInstallChunkBuildLayer
{
    friend class BuildImageInstallChunk;

public:
    BuildImageInstallChunkBuildLayer(BuildLayerRoot* root = NIL, BuildImageInstallChunk* installChunk = NIL)
        : m_root(root)
        , m_installChunk(installChunk)
    {}

    const BuildLayerRoot& getRoot() const { return *m_root; }

    const BuildImageInstallChunk& getInstallChunk() const { return *m_installChunk; }
    template<class T> const T& getInstallChunk() const { return *(const T*)m_installChunk; }
    BuildImageInstallChunk& getInstallChunk() { return *m_installChunk; }
    template<class T> T& getInstallChunk() { return *(T*)m_installChunk; }

private:
    BuildLayerRoot* m_root;
    BuildImageInstallChunk* m_installChunk;
};

class BuildImageInstallChunk
{
    friend class BuildImageSuperbundle;
    friend class BuildImageInstallManifest;

public:
    static constexpr u32 c_maxBuildLayers = 2; // base, patch
    typedef fixed_vector<BuildImageInstallChunkBuildLayer, c_maxBuildLayers> InstallChunkBuildLayers_t;
    typedef Vec<BuildImageSuperbundle*> Superbundles_t;

public:
    BuildImageInstallChunk(BuildImageInstallManifest* manifest, const Guid& id = c_emptyGuid, uint index = 0)
        : m_installManifest(*manifest)
        , m_catalogName("")
        , m_guid(id)
        , m_installBundle("")
        , m_index(index)
    {}
    virtual ~BuildImageInstallChunk() = default;

    virtual void parse(const DbObject& installChunkSpec);

    const char* getCatalogName() const { return m_catalogName; }
    const Guid& getGuid() const { return m_guid; }
    const char* getInstallBundle() const { return m_installBundle; }
    uint getIndex() const { return m_index; }

    FB_STORAGE_API void setInstallPackage(const char* installPackage);
    void setCatalogName(const char* catalogName) { m_catalogName = catalogName; }
    void setInstallBundle(const char* installBundle) { m_installBundle = installBundle; }

    const BuildImageInstallManifest& getInstallManifest() const { return m_installManifest; }
    template<class T> const T& getInstallManifest() const { return (const T&)m_installManifest; }
    BuildImageInstallManifest& getInstallManifest() { return m_installManifest; }
    template<class T> T& getInstallManifest() { return (T&)m_installManifest; }

    const InstallChunkBuildLayers_t& getBuildLayers() const { return m_buildLayers; }
    InstallChunkBuildLayers_t& getBuildLayers() { return m_buildLayers; }

    FB_STORAGE_API const BuildLayerRoot* getBuildLayer(uint index) const;
    FB_STORAGE_API void addBuildLayer(BuildLayerRoot* root);

    /*
        Note: Don't add superbundles from install chunks, either do it from the superbundle itself.
    */
    const Superbundles_t& getSuperbundles() const { return m_superbundles; }
    Superbundles_t& getSuperbundles() { return m_superbundles; }
    
    template<class T>
    const BuildImageRangeIterator<T> getSuperbundles() const
        { return m_superbundles; }
    
    template<class T>
    BuildImageRangeIterator<T> getSuperbundles()
        { return m_superbundles; }

private:
    BuildImageInstallManifest& m_installManifest;
    InstallChunkBuildLayers_t m_buildLayers;

    const char* m_catalogName;
    Guid m_guid;
    const char* m_installBundle;
    uint m_index;

    Superbundles_t m_superbundles;
};

class BuildImageInstallGroup
{
public:
    friend class BuildImageInstallChunk;

public:
    BuildImageInstallGroup(BuildImageInstallManifest* manifest)
        : m_installManifest(*manifest)
    {}

    const BuildImageInstallManifest& getInstallManifest() const { return m_installManifest; }
    template<class T> const T& getInstallManifest() const { return (const T&)m_installManifest; }
    BuildImageInstallManifest& getInstallManifest() { return m_installManifest; }
    template<class T> T& getInstallManifest() { return (T&)m_installManifest; }

    eastl::string name;
    Vec<BuildImageInstallChunk*> chunks;

private:
    BuildImageInstallManifest& m_installManifest;
};

class BuildImageInstallManifest
{
    friend class BuildImageLayout;

public:
    typedef Vec<ScopedRef<BuildImageInstallChunk>> InstallChunks_t;
    typedef Dict<Guid, BuildImageInstallChunk*> InstallChunksByGuid;

    typedef Vec<ScopedRef<BuildImageInstallGroup>> InstallGroups_t;
    typedef Dict<const char*, BuildImageInstallGroup*> InstallGroupsByName;

public:
    BuildImageInstallManifest(BuildImageLayout* layout = nullptr)
        : m_layout(layout)
    {}

    ~BuildImageInstallManifest() = default;

    virtual void parse(const DbObject& installManifestSpec);

    const BuildImageLayout& getLayout() const { return *m_layout; }
    BuildImageLayout& getLayout() { return *m_layout; }

    const InstallChunks_t& getInstallChunks() const { return m_installChunks; }
    InstallChunks_t& getInstallChunks() { return m_installChunks; }
    
    template<class T>
    const BuildImageRangeIterator<T> getInstallChunks() const
        { return m_installChunks; }
    
    template<class T>
    BuildImageRangeIterator<T> getInstallChunks()
        { return m_installChunks; }

    const InstallGroups_t& getInstallGroups() const { return m_installGroups; }
    InstallGroups_t& getInstallGroups() { return m_installGroups; }

    template<class T>
    const BuildImageRangeIterator<T> getInstallGroups() const
        { return m_installGroups; }
    
    template<class T>
    BuildImageRangeIterator<T> getInstallGroups()
        { return m_installGroups; }

    FB_STORAGE_API BuildImageInstallChunk& createInstallChunk(const Guid& id);
    FB_STORAGE_API const BuildImageInstallChunk* lookupInstallChunk(const Guid& id) const;
    FB_STORAGE_API BuildImageInstallChunk* lookupInstallChunk(const Guid& id);

    template<typename T>
    inline T* lookupInstallChunk(const Guid& id)
        { return reinterpret_cast<T*>(lookupInstallChunk(id)); }
    
    template<typename T>
    inline const T* lookupInstallChunk(const Guid& id) const
        { return reinterpret_cast<T*>(lookupInstallChunk(id)); }

    FB_STORAGE_API BuildImageInstallGroup& createInstallGroup(eastl::string name);
    FB_STORAGE_API const BuildImageInstallGroup* lookupInstallGroup(eastl::string name) const;
    FB_STORAGE_API BuildImageInstallGroup* lookupInstallGroup(eastl::string name);

    template<typename T>
    inline T* lookupInstallGroup(const eastl::string& name)
        { return reinterpret_cast<T*>(lookupInstallGroup(name)); }
    
    template<typename T>
    inline const T* lookupInstallGroup(const eastl::string& name) const
        { return reinterpret_cast<T*>(lookupInstallGroup(name)); }

private:
    BuildImageLayout* m_layout;

    InstallChunks_t m_installChunks;
    InstallChunksByGuid m_installChunksByGuid;

    InstallGroups_t m_installGroups;
    InstallGroupsByName m_installGroupsByName;
};

class BuildImageLayout
{
public:
    using InstallChunks_t = BuildImageInstallManifest::InstallChunks_t;
    using InstallGroups_t = BuildImageInstallManifest::InstallGroups_t;
    typedef Vec<BuildImageSuperbundle*> Superbundles_t;
    typedef Dict<const char*, BuildImageSuperbundle*, HashStringNoCase> SuperbundlesByName;

public:
    BuildImageLayout(BuildImage* buildImage = NIL)
        : m_buildImage(*buildImage)
    {
        m_installManifest.m_layout = this;
    }

    virtual ~BuildImageLayout() = default;

    virtual void parse(const DbObject& layoutSpec);

    const BuildImage& getBuildImage() const { return m_buildImage; }
    template<class T> const T& getBuildImage() const { return (const T&)m_buildImage; }
    BuildImage& getBuildImage() { return m_buildImage; }
    template<class T> T& getBuildImage() { return (T&)m_buildImage; }

    const BuildImageInstallManifest& getInstallManifest() const { return m_installManifest; }
    template<class T> const T& getInstallManifest() const { return (const T&)m_installManifest; }
    BuildImageInstallManifest& getInstallManifest() { return m_installManifest; }
    template<class T> T& getInstallManifest() { return (T&)m_installManifest; }

    /* Superbundles */
    const Superbundles_t& getSuperbundles() const { return m_superbundles; }
    Superbundles_t& getSuperbundles() { return m_superbundles; }

    FB_STORAGE_API BuildImageSuperbundle& createSuperbundle(const char* superbundleName);
    FB_STORAGE_API const BuildImageSuperbundle* lookupSuperbundle(const char* superbundleName) const;
    FB_STORAGE_API BuildImageSuperbundle* lookupSuperbundle(const char* superbundleName);

    template<typename T>
    inline T* lookupSuperbundle(const char* sbName)
        { return reinterpret_cast<T*>(lookupSuperbundle(sbName)); }
    
    template<typename T>
    inline const T* lookupSuperbundle(const char* sbName) const
        { return reinterpret_cast<T*>(lookupSuperbundle(sbName)); }

    /* Install chunks */
    const InstallChunks_t& getInstallChunks() const { return m_installManifest.getInstallChunks(); }
    InstallChunks_t& getInstallChunks() { return m_installManifest.getInstallChunks(); }

    /* Install groups */
    const InstallGroups_t& getInstallGroups() const { return m_installManifest.getInstallGroups(); }
    InstallGroups_t& getInstallGroups() { return m_installManifest.getInstallGroups(); }

    FB_STORAGE_API BuildImageInstallChunk& createInstallChunk(const Guid& id);
    FB_STORAGE_API const BuildImageInstallChunk* lookupInstallChunk(const Guid& id) const;
    FB_STORAGE_API BuildImageInstallChunk* lookupInstallChunk(const Guid& id);

    template<typename T>
    inline T* lookupInstallChunk(const Guid& id)
        { return reinterpret_cast<T*>(lookupInstallChunk(id)); }
    
        template<typename T>
    inline const T* lookupInstallChunk(const Guid& id) const
        { return reinterpret_cast<T*>(lookupInstallChunk(id)); }

    FB_STORAGE_API BuildImageInstallGroup& createInstallGroup(eastl::string name);
    FB_STORAGE_API const BuildImageInstallGroup* lookupInstallGroup(eastl::string name) const;
    FB_STORAGE_API BuildImageInstallGroup* lookupInstallGroup(eastl::string name);

    template<typename T>
    T* lookupInstallGroup(const eastl::string& name)
        { return reinterpret_cast<T*>(lookupInstallGroup(name)); }
    
    template<typename T>
    const T* lookupInstallGroup(const eastl::string& name) const
        { return reinterpret_cast<T*>(lookupInstallGroup(name)); }

private:
    BuildImage& m_buildImage;
    BuildImageInstallManifest m_installManifest;
    SuperbundlesByName m_superbundlesByName;
    Superbundles_t m_superbundles;
};

class BuildImage
{
    typedef fixed_vector<BuildLayerRoot, 2> Roots_t;

public:
    // @param mount: The mount name ( eg.: /native_data )
    // @param dir: ABSOLUTE PATH IS REQUIRED, RESOLVE IT YOURSELVES.
    FB_STORAGE_API BuildImage(VirtualFileSystem* vfs, const char* mount, const char* dir);
    FB_STORAGE_API ~BuildImage();

    const BuildImageLayout& getLayout() const { return *m_layout.get(); }
    template<class T> const T& getLayout() const { return *(const T*)m_layout.get(); }
    BuildImageLayout& getLayout() { return *m_layout.get(); }
    template<class T> T& getLayout() { return *(T*)m_layout.get(); }
    
    // This is relative from the image dir (don't end it with a path separator).
    FB_STORAGE_API BuildLayerRoot& createRoot(BuildLayerRootType rootType, const char* dir);
    const Roots_t& getRoots() const { return m_roots; }
    Roots_t& getRoots() { return m_roots; }

private:
    VirtualFileSystem* m_vfs;
    const char* m_mount;
    const char* m_dir;
    ScopedRef<BuildImageLayout> m_layout;
    Roots_t m_roots;
};
}
