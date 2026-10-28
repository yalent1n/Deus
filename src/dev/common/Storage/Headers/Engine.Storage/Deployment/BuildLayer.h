#pragma once
#include "../Bundle.h"
#include <Engine.Base/Ref.h>
#include <Engine.Base/Database/DbObject.h>
#include <Engine.Core/VirtualFileSystem.h>

namespace rw
{
enum class BuildLayerRootType : u8
{
    Base,
    Patch,
    Additional
};

class BuildLayerFile;
class BuildLayerRoot
{
public:
    // @param mount: Mounted backend of root directory
    // @param layout: Virtual path of layout file
    // @param rootType: Type of root
    FB_STORAGE_API BuildLayerRoot(FsBackend* mount = nullptr, const char* layout = nullptr, BuildLayerRootType rootType = BuildLayerRootType::Base);
    FB_STORAGE_API ~BuildLayerRoot();

    FB_STORAGE_API const char* getRoot() const;
    FB_STORAGE_API const char* getLayout() const;

    FB_STORAGE_API BuildLayerRootType getRootType() const;

    FB_STORAGE_API SharedBuffer*    shareFile(const BuildLayerFile& file, bool check = true) const;
    FB_STORAGE_API Buffer*          openFile(const BuildLayerFile& file) const;
    FB_STORAGE_API Buffer*          createFile(const BuildLayerFile& file, bool append = false, bool read = true) const;
    FB_STORAGE_API bool             copyFile(const BuildLayerFile& srcFile, const BuildLayerFile& dstFile) const;
    FB_STORAGE_API bool             moveFile(const BuildLayerFile& oldFile, const BuildLayerFile& newFile) const;
    FB_STORAGE_API bool             sealFile(const BuildLayerFile& file) const;
    FB_STORAGE_API bool             unsealFile(const BuildLayerFile& file) const;

    FB_STORAGE_API DbObject         readDbObject(const BuildLayerFile& file) const;
    FB_STORAGE_API bool             writeDbObject(const BuildLayerFile& file, const DbObject& o) const;

private:
    FsBackend* m_mount;
    const char* m_layout;
    BuildLayerRootType m_rootType;
};

class BuildLayerFile
{
public:
    FB_STORAGE_API BuildLayerFile(const BuildLayerRoot& root);
    FB_STORAGE_API virtual ~BuildLayerFile();

    FB_STORAGE_API const BuildLayerRoot& getRoot() const;

    virtual const char* getPath(StringEditor& path) const = NIL;
    virtual const char* getFilePath(StringEditor& path) const = NIL;
    
private:
    const BuildLayerRoot& m_root;
};

class BuildLayerFileFullPath : public BuildLayerFile
{
public:
    FB_STORAGE_API BuildLayerFileFullPath(const BuildLayerRoot& root, const char* path);
    FB_STORAGE_API virtual ~BuildLayerFileFullPath();

    FB_STORAGE_API const char* getPath(StringEditor& path) const override;
    FB_STORAGE_API const char* getFilePath(StringEditor& path) const override;

private:
    const char* m_path;
};

class BuildLayerFileLayout : public BuildLayerFileFullPath
{
public:
    BuildLayerFileLayout(const BuildLayerRoot& root)
        : BuildLayerFileFullPath(root, "layout.toc")
    {}
};
}