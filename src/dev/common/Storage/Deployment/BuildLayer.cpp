#include <Engine.Storage/Deployment/BuildLayer.h>

namespace rw
{
BuildLayerRoot::BuildLayerRoot(FsBackend* mount, const char* layout, BuildLayerRootType rootType)
    : m_mount(mount)
    , m_layout(layout)
    , m_rootType(rootType)
{
    //Assertf(mount, "Build layer %u has no mount point", uint(rootType));
}

BuildLayerRoot::~BuildLayerRoot()
{

}

const char* BuildLayerRoot::getRoot() const
{
    return m_mount->directory();
}

const char* BuildLayerRoot::getLayout() const
{
    return m_layout;
}

BuildLayerRootType BuildLayerRoot::getRootType() const
{
    return m_rootType;
}

SharedBuffer* BuildLayerRoot::shareFile(const BuildLayerFile& file, bool check) const
{
    PathStringBuilder path;
    file.getFilePath(path);
    return m_mount->shareFile(c_str(path), check);
}

Buffer* BuildLayerRoot::openFile(const BuildLayerFile& file) const
{
    PathStringBuilder path;
    file.getFilePath(path);
    return m_mount->openFile(c_str(path));
}

Buffer* BuildLayerRoot::createFile(const BuildLayerFile& file, bool append, bool read) const
{
    PathStringBuilder path;
    file.getFilePath(path);
    return m_mount->createFile(c_str(path), append, read);
}

bool BuildLayerRoot::copyFile(const BuildLayerFile& srcFile, const BuildLayerFile& dstFile) const
{
    PathStringBuilder srcPath;
    srcFile.getFilePath(srcPath);
    
    PathStringBuilder dstPath;
    dstFile.getFilePath(dstPath);

    return m_mount->copyFile(c_str(srcPath), c_str(dstPath));
}

bool BuildLayerRoot::moveFile(const BuildLayerFile& oldFile, const BuildLayerFile& newFile) const
{
    PathStringBuilder oldPath;
    oldFile.getFilePath(oldPath);
    
    PathStringBuilder newPath;
    newFile.getFilePath(newPath);

    return m_mount->moveFile(c_str(oldPath), c_str(newPath));
}

bool BuildLayerRoot::sealFile(const BuildLayerFile& file) const
{
    PathStringBuilder path;
    file.getFilePath(path);
    return m_mount->sealFile(c_str(path));
}

bool BuildLayerRoot::unsealFile(const BuildLayerFile& file) const
{
    PathStringBuilder path;
    file.getFilePath(path);
    return m_mount->unsealFile(c_str(path));
}

#pragma pack(push, 1)
struct PropFile
{
    static constexpr u32 c_magic = make_cc(0xD1, 0xCE, 0xDA, 0xBA);
    
    u32 magic;
    DbOid oid;
};
#pragma pack(pop)

DbObject BuildLayerRoot::readDbObject(const BuildLayerFile& file) const
{
    ScopedRef buffer(openFile(file));
    if(buffer.isNull())
        return c_NullObject;

    PropFile propFile;
    if(buffer->read(&propFile, sizeof propFile) < E_BIO_SUCCESS)
        return c_NullObject;
    
    if(propFile.magic != PropFile::c_magic)
        return c_NullObject;

    StringBuilder<128> objectId;
    file.getPath(objectId);
    objectId << " (ID: ";
    propFile.oid.toString(objectId);
    objectId << ')';

    StreamSize objectSize = buffer->getAvailableBytes() - sizeof(PropFile);
    u8* object = (u8*) HeapAllocGlobal(objectSize);
    if(buffer->read(object, objectSize) < E_BIO_SUCCESS)
        return c_NullObject;

    ELOGF("Read properties - %s", c_str(objectId));
    return DbObject { object };
}

bool BuildLayerRoot::writeDbObject(const BuildLayerFile& file, const DbObject& o) const
{
    PropFile propFile;
    propFile.magic = PropFile::c_magic;
    propFile.oid.generate();

    ScopedRef buffer(createFile(file));
    if(buffer.isNull())
        return false;

    if(buffer->write(&propFile, sizeof propFile) < E_BIO_SUCCESS)
        return false;

    if(buffer->write(o.data(), o.dataSize()) < E_BIO_SUCCESS)
        return false;

    StringBuilder<128> objectId;
    file.getFilePath(objectId);
    objectId << " (ID: ";
    propFile.oid.toString(objectId);
    objectId << ')';
    ELOGF("Write properties - %s", c_str(objectId));

    return true;
}

BuildLayerFile::BuildLayerFile(const BuildLayerRoot& root)
    : m_root(root)
{}

BuildLayerFile::~BuildLayerFile()
{}

const BuildLayerRoot& BuildLayerFile::getRoot() const
{
    return m_root;
}

BuildLayerFileFullPath::BuildLayerFileFullPath(const BuildLayerRoot& root, const char* path)
    : BuildLayerFile(root)
    , m_path(path)
{}

BuildLayerFileFullPath::~BuildLayerFileFullPath()
{}

const char* BuildLayerFileFullPath::getPath(StringEditor& path) const
{
    path << m_path;
    return m_path;
}

const char* BuildLayerFileFullPath::getFilePath(StringEditor& path) const
{
    path << getRoot().getRoot() << m_path;
    return c_str(path);
}
}