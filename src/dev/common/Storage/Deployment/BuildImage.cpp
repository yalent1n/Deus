#include <Engine.Storage/Deployment/BuildImage.h>

namespace rw
{
void BuildImageSuperbundle::parse(const DbObject& superbundleSpec)
{
    // nop, currently.
}

const char* BuildImageSuperbundle::getName() const
{
    return m_name;
}

u32 BuildImageSuperbundle::getIndex() const
{
    return m_index;
}

u32 BuildImageSuperbundle::getHash() const
{
    const void* HashD = isnil(m_name) ? nullptr : m_name;
    const size_t HashS = isnil(m_name) ? 0 : strlen(m_name);
    return DJB(HashD, HashS);
}

BuildImageSuperbundleInstallChunk& BuildImageSuperbundle::createSuperbundleInstallChunk(SplitSuperbundleType sbSplitType, BuildImageInstallChunk& ic)
{
    BuildImageSuperbundleInstallChunk* sbic = new BuildImageSuperbundleInstallChunk(this, sbSplitType);
    new (&m_superbundleInstallChunks.push_back()) ScopedRef<BuildImageSuperbundleInstallChunk>(sbic);
    sbic->m_installChunks.push_back(&ic);
    ic.m_superbundles.push_back(this);
    ELOGF("Attached superbundle \"%s\" to install chunk \"%s\"", getName(), GuidStr(ic.getGuid()).str);
    return *sbic;
}

void BuildImageInstallChunk::parse(const DbObject& installChunkSpec)
{
    if(installChunkSpec.isNullOrEmpty())
        return;

    m_catalogName = installChunkSpec["name"].valueString("");
    m_installBundle = installChunkSpec["installBundle"].valueString("");

    auto superbundles = installChunkSpec["superbundles"].valueArray();
    for(DbObjectElementIterator sbit(superbundles); sbit.more();)
    {
        auto sb = sbit.next().valueString("");
        if(BuildImageSuperbundle* superbundle = getInstallManifest().getLayout().lookupSuperbundle(sb)) // the superbundle has to be parsed before from the layout
            superbundle->createSuperbundleInstallChunk(SplitSuperbundleType::None, *this);
    }
}

void BuildImageInstallChunk::setInstallPackage(const char* installPackage)
{
    if(isnil(installPackage))
        return;

    StringBuilder<> installBundle;
    installBundle << getPlatformName(CurrentPlatform) << '/' << installPackage;
    setlower(installBundle.data(), installBundle.size());

    StringBuilder<> catalogName;
    catalogName << installPackage << "/cas.cat";

    m_installBundle = installBundle.detach();
    m_catalogName = catalogName.detach();
    ELOGF("Install bundle: %s, Catalog: %s", m_installBundle, m_catalogName);
}

const BuildLayerRoot* BuildImageInstallChunk::getBuildLayer(uint index) const
{
    const BuildImageInstallChunkBuildLayer& icBuildLayer = m_buildLayers[index];
    return &icBuildLayer.getRoot();
}

void BuildImageInstallChunk::addBuildLayer(BuildLayerRoot* root)
{
    uint rootIndex = uint(root->getRootType());
    BuildImageInstallChunkBuildLayer& icBuildLayer = m_buildLayers[rootIndex];
    icBuildLayer.m_installChunk = this;
    icBuildLayer.m_root = root;
}

void BuildImageInstallManifest::parse(const DbObject& installManifestSpec)
{
    for(DbObjectElementIterator it(installManifestSpec["installChunks"].valueArray()); it.more();)
    {
        DbObject installChunkSpec = it.next().valueObject();
        BuildImageInstallChunk& installChunk = createInstallChunk(installChunkSpec["id"].valueGuid());
        installChunk.parse(installChunkSpec);
    }

}

// Install chunks
BuildImageInstallChunk& BuildImageInstallManifest::createInstallChunk(const Guid& id)
{
    InstallChunksByGuid::iterator foundIt = m_installChunksByGuid.find(id);
    if(foundIt != m_installChunksByGuid.end())
        return *foundIt->second;

    uint index = m_installChunks.size();
    BuildImageInstallChunk* installChunk = new BuildImageInstallChunk(this, id, index);
    ELOGF("New install chunk \"%s\" at %u", GuidStr(installChunk->getGuid()).str, index);

    m_installChunks.push_back() = installChunk;
    m_installChunksByGuid.insert(id, installChunk);
    return *installChunk;
}

const BuildImageInstallChunk* BuildImageInstallManifest::lookupInstallChunk(const Guid& id) const
{
    InstallChunksByGuid::iterator foundIt = m_installChunksByGuid.find(id);
    return foundIt != m_installChunksByGuid.end() ? foundIt->second : nullptr;
}

BuildImageInstallChunk* BuildImageInstallManifest::lookupInstallChunk(const Guid& id)
{
    InstallChunksByGuid::iterator foundIt = m_installChunksByGuid.find(id);
    return foundIt != m_installChunksByGuid.end() ? foundIt->second : nullptr;
}

// Install groups
BuildImageInstallGroup& BuildImageInstallManifest::createInstallGroup(eastl::string name)
{
    InstallGroupsByName::iterator foundIt = m_installGroupsByName.find(name);
    if(foundIt != m_installGroupsByName.end())
        return *foundIt->second;

    BuildImageInstallGroup* installGroup = new BuildImageInstallGroup(this);
    installGroup->name = name;
    ELOGF("New install group \"%s\"", installGroup->name.c_str());

    m_installGroups.push_back() = installGroup;
    m_installGroupsByName.insert(name.c_str(), installGroup);
    return *installGroup;
}

const BuildImageInstallGroup* BuildImageInstallManifest::lookupInstallGroup(eastl::string name) const
{
    InstallGroupsByName::iterator foundIt = m_installGroupsByName.find(name);
    return foundIt != m_installGroupsByName.end() ? foundIt->second : nullptr;
}

BuildImageInstallGroup* BuildImageInstallManifest::lookupInstallGroup(eastl::string name)
{
    InstallGroupsByName::iterator foundIt = m_installGroupsByName.find(name);
    return foundIt != m_installGroupsByName.end() ? foundIt->second : nullptr;
}

void BuildImageLayout::parse(const DbObject& layoutSpec)
{
    for(DbObjectElementIterator it(layoutSpec["superbundles"].valueArray()); it.more();)
    {
        DbObject superbundleSpec = it.next().valueObject();
        BuildImageSuperbundle& sb = createSuperbundle(superbundleSpec["name"].valueString(""));
        sb.parse(superbundleSpec);
    }

    if(auto installManifestSpecEl = layoutSpec["installManifest"])
    {
        getInstallManifest().parse(installManifestSpecEl.valueObject());
    }
}

BuildImageSuperbundle& BuildImageLayout::createSuperbundle(const char* superbundleName)
{
    __builtin_assert(!isnil(superbundleName));
    SuperbundlesByName::iterator foundIt = m_superbundlesByName.find(superbundleName);
    if(foundIt != m_superbundlesByName.end())
        return *foundIt->second;

    uint index = m_superbundles.size();
    BuildImageSuperbundle* sb = new BuildImageSuperbundle(this, superbundleName, index);
    ELOGF("New superbundle \"%s\" at %u", sb->getName(), index);

    m_superbundles.push_back() = sb;
    m_superbundlesByName.insert(superbundleName, sb);
    return *sb;
}

const BuildImageSuperbundle* BuildImageLayout::lookupSuperbundle(const char* superbundleName) const
{
    SuperbundlesByName::iterator foundIt = m_superbundlesByName.find(superbundleName);
    return foundIt != m_superbundlesByName.end() ? foundIt->second : nullptr;
}

BuildImageSuperbundle* BuildImageLayout::lookupSuperbundle(const char* superbundleName)
{
    SuperbundlesByName::iterator foundIt = m_superbundlesByName.find(superbundleName);
    return foundIt != m_superbundlesByName.end() ? foundIt->second : nullptr;
}

BuildImageInstallChunk& BuildImageLayout::createInstallChunk(const Guid& id)
{
    return m_installManifest.createInstallChunk(id);
}

const BuildImageInstallChunk* BuildImageLayout::lookupInstallChunk(const Guid& id) const
{
    return m_installManifest.lookupInstallChunk(id);
}

BuildImageInstallChunk* BuildImageLayout::lookupInstallChunk(const Guid& id)
{
    return m_installManifest.lookupInstallChunk(id);
}

BuildImageInstallGroup& BuildImageLayout::createInstallGroup(eastl::string name)
{
    return m_installManifest.createInstallGroup(name);
}

const BuildImageInstallGroup* BuildImageLayout::lookupInstallGroup(eastl::string name) const
{
    return m_installManifest.lookupInstallGroup(name);
}

BuildImageInstallGroup* BuildImageLayout::lookupInstallGroup(eastl::string name)
{
    return m_installManifest.lookupInstallGroup(name);
}

BuildImage::BuildImage(VirtualFileSystem* vfs, const char* mount, const char* dir)
    : m_vfs(vfs)
    , m_mount(mount)
    , m_dir(dir)
    , m_layout(new BuildImageLayout(this))
    {
    __builtin_assert(!isnil(mount) && !isnil(dir));
    ELOGF("<Build Image> Located at %s, Mounted at %s", m_dir, m_mount);
}

BuildImage::~BuildImage()
{
}

BuildLayerRoot& BuildImage::createRoot(BuildLayerRootType rootType, const char* dir)
{
    StringBuilder<> rootMountName;
    rootMountName << m_mount << '/' << dir;
    uint rootIndex = uint(rootType);
    ELOGF("New root at %u : %s", rootIndex, c_str(rootMountName));

    PathStringBuilder rootMountPath;
    rootMountPath << m_dir << dir << '\\';

    ELOGF("<Build Image> Root located at %s, Mounted at %s", c_str(rootMountPath), c_str(rootMountName));

    FsBackend* rootMount = m_vfs->mount(c_str(rootMountName), rootMountPath.detach());

    BuildLayerRoot& root = *(new (&m_roots[rootIndex]) BuildLayerRoot(rootMount, "", rootType));
    return root;
}
}
