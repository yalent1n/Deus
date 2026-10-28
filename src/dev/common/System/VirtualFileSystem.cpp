#include <Engine.Core/VirtualFileSystem.h>
#include <Engine.Core/Backends/Win32FsBackend.h>
#include <Engine.Base/String/Util.h>
#include <stdio.h>

namespace rw
{
VirtualFileSystem::VirtualFileSystem()
    : m_mounts()
    , m_systemBackend(nullptr)
{
    m_systemBackend = mount("", "");
    m_mounts.erase(m_mounts.begin());
}

VirtualFileSystem::~VirtualFileSystem()
{

}

Buffer* VirtualFileSystem::touch(const char* path, BIOACCESS access)
{
    if(isnil(path))
        return NIL;

    const char* mount = nullptr;
    FsBackend* fs = findBackend(path, &mount);

    if(fs && mount)
    {
        size_t len = strlen(mount);
        if(!len)
            return nullptr;

        path += len + 1;
        return fs->touch(path, BIOACCESS(access));
    }

    return m_systemBackend->touch(path, BIOACCESS(access));
}

Buffer* VirtualFileSystem::openFile(const char* path, bool check)
{
    u16 caps = E_BIO_READ|E_BIO_SHARE_READ | (!check ? E_BIO_ALWAYS_CREATE : 0);
    return touch(path, BIOACCESS(caps));
}

Buffer* VirtualFileSystem::createFile(const char* path, bool append, bool read)
{
    u16 caps = E_BIO_WRITE|E_BIO_SHARE_READ | (read ? E_BIO_READ : 0) | (append ? E_BIO_APPEND : 0);
    return touch(path, BIOACCESS(caps));
}

SharedBuffer* VirtualFileSystem::shareFile(const char* path, bool check)
{
    if(isnil(path))
        return NIL;
    
    const char* mount = nullptr;
    FsBackend* fs = findBackend(path, &mount);

    // fix compareIgnoreCase to check WITH the length of the iterator's prefix
    if(fs && mount)
    {
        size_t len = strlen(mount);
        if(!len)
            return nullptr;

        path += len + 1;
        return fs->shareFile(path, check);
    }

    return m_systemBackend->shareFile(path, check);
}

bool VirtualFileSystem::deleteFile(const char* path)
{
    if(isnil(path))
        return NIL;
    
    const char* mount = nullptr;
    FsBackend* fs = findBackend(path, &mount);

    // fix compareIgnoreCase to check WITH the length of the iterator's prefix
    if(fs && mount)
    {
        size_t len = strlen(mount);
        if(!len)
            return false;

        path += len + 1;
        return fs->deleteFile(path);
    }

    return m_systemBackend->deleteFile(path);
}

bool VirtualFileSystem::copyFile(const char* src, const char* dst)
{
    if(isnil(src) || isnil(dst))
        return NIL;
    
    PathStringBuilder nativeSrcPath;
    const char* srcMount = nullptr;
    FsBackend* srcFs = findBackend(src, &srcMount);
    
    PathStringBuilder nativeDstPath;
    const char* dstMount = nullptr;
    FsBackend* dstFs = findBackend(dst, &dstMount);
    
    if(srcFs && srcMount)
    {
        size_t len = strlen(srcMount);
        if(!len)
            return false;

        src += len + 1;
        srcFs->nativePathTranslation(src, nativeSrcPath);
    }
    else
    {
        nativeSrcPath << src;
    }
    
    if(dstFs && dstMount)
    {
        size_t len = strlen(dstMount);
        if(!len)
            return false;

        dst += len + 1;
        dstFs->nativePathTranslation(dst, nativeDstPath);
    }
    else
    {
        nativeDstPath << dst;
    }    
    
    return m_systemBackend->copyFile(c_str(nativeSrcPath), c_str(nativeDstPath));
}

bool VirtualFileSystem::moveFile(const char* src, const char* dst)
{
    if(isnil(src) || isnil(dst))
        return NIL;
    
    PathStringBuilder nativeSrcPath;
    const char* srcMount = nullptr;
    FsBackend* srcFs = findBackend(src, &srcMount);
    
    PathStringBuilder nativeDstPath;
    const char* dstMount = nullptr;
    FsBackend* dstFs = findBackend(dst, &dstMount);
    
    if(srcFs && srcMount)
    {
        size_t len = strlen(srcMount);
        if(!len)
            return false;

        src += len + 1;
        srcFs->nativePathTranslation(src, nativeSrcPath);
    }
    else
    {
        nativeSrcPath << src;
    }
    
    if(dstFs && dstMount)
    {
        size_t len = strlen(dstMount);
        if(!len)
            return false;

        dst += len + 1;
        dstFs->nativePathTranslation(dst, nativeDstPath);
    }
    else
    {
        nativeDstPath << dst;
    }    
    
    return m_systemBackend->moveFile(c_str(nativeSrcPath), c_str(nativeDstPath));
}

bool VirtualFileSystem::sealFile(const char* path)
{
    if(isnil(path))
        return false;

    const char* mount = nullptr;
    FsBackend* fs = findBackend(path, &mount);

    if(fs && mount)
    {
        size_t len = strlen(mount);
        if(!len)
            return false;

        path += len + 1;
        return fs->sealFile(path);
    }

    return m_systemBackend->sealFile(path);
}

bool VirtualFileSystem::unsealFile(const char* path)
{
    if(isnil(path))
        return false;

    const char* mount = nullptr;
    FsBackend* fs = findBackend(path, &mount);

    if(fs && mount)
    {
        size_t len = strlen(mount);
        if(!len)
            return false;

        path += len + 1;
        return fs->unsealFile(path);
    }

    return m_systemBackend->unsealFile(path);
}

bool VirtualFileSystem::fileExists(const char* path)
{
    if(isnil(path))
        return false;

    const char* mount = nullptr;
    FsBackend* fs = findBackend(path, &mount);

    if(fs && mount)
    {
        size_t len = strlen(mount);
        if(!len)
            return false;

        path += len + 1;
        return fs->fileExists(path);
    }

    return m_systemBackend->fileExists(path);
}

bool VirtualFileSystem::createDirectory(const char* path) const
{
    if(isnil(path))
        return false;

    const char* mount = nullptr;
    FsBackend* fs = findBackend(path, &mount);

    if(fs && mount)
    {
        size_t len = strlen(mount);
        if(!len)
            return false;

        path += len + 1;
        return fs->createDirectory(path);
    }

    return m_systemBackend->createDirectory(path);
}

bool VirtualFileSystem::deleteDirectory(const char* path) const
{
    if(isnil(path))
        return false;

    const char* mount = nullptr;
    FsBackend* fs = findBackend(path, &mount);

    if(fs && mount)
    {
        size_t len = strlen(mount);
        if(!len)
            return false;

        path += len + 1;
        return fs->deleteDirectory(path);
    }

    return m_systemBackend->deleteDirectory(path);
}

FileIterator* VirtualFileSystem::getFileIterator(const char* path) const
{
    if(isnil(path))
        return nullptr;

    const char* mount = nullptr;
    FsBackend* fs = findBackend(path, &mount);

    if(fs && mount)
    {
        size_t len = strlen(mount);
        if(!len)
            return nullptr;

        path += len + 1;
        return fs->getFileIterator(path);
    }

    return m_systemBackend->getFileIterator(path);
}

void VirtualFileSystem::traverseFiles(const char* filter, TraverseFn_t fn, bool excludeDirs)
{
    if(isnil(filter))
        return;

    const char* mount = nullptr;
    FsBackend* fs = findBackend(filter, &mount);

    if(fs && mount)
    {
        size_t len = strlen(mount);
        if(!len)
            return;

        filter += len + 1;
        fs->traverseFiles(filter, fn, excludeDirs);
        return;
    }

    m_systemBackend->traverseFiles(filter, fn, excludeDirs);
}

FsBackend* VirtualFileSystem::registerBackend(FsBackend* view, const char* viewIndex)
{
    if(FsBackend* tmp = findBackend(viewIndex); tmp)
        return tmp;

    //ELOGF("Registering view %s @ %s", viewIndex, view->directory());
    (void) m_mounts.push_back({viewIndex, view});
    return view;
}

FsBackend* VirtualFileSystem::mount(const char* mountName, StringFn_t&& dirFn)
{
    FsBackend* backend = NIL;
    if(backend = findBackend(mountName); backend)
        return backend;

#ifdef SvcCompilerMSVC
    backend = new WindowsFS(std::move(dirFn));
#else
    //view = new UnixIoBackend(dirFn);
#endif

    m_mounts.push_back({mountName, backend});
    return backend;
}

FsBackend* VirtualFileSystem::mount(const char* mountName, const char* dirAbs)
{
    FsBackend* backend = NIL;
    if(backend = findBackend(mountName); backend != m_systemBackend)
        return backend;

#ifdef SvcCompilerMSVC
    backend = new WindowsFS(dirAbs);
#else
    //view = new UnixIoBackend(dirFn);
#endif

    m_mounts.push_back({mountName, backend});
    return backend;
}

FsBackend* VirtualFileSystem::findBackend(const char* mountName, const char** mountResult) const
{    
    if(mountResult)
        *mountResult = "";

    if(!m_mounts.empty())
    {
        for(Cache::const_iterator i = m_mounts.begin(), e = m_mounts.end(); i != e; i++)
        {
            if(equalsIgnoreCase(i->id, mountName, strlen(i->id)))
            {
                if(mountResult)
                    *mountResult = i->id;

                return i->backend;
            }
        }
    }

    if(mountResult)
        *mountResult = "";

    return m_systemBackend;
}

BIORETURN VirtualFileSystem::unmount(FsBackend* backend)
{
    if(!backend)
        return E_BIO_ERR_ARGUMENT;

    Cache::iterator i = nullptr, e = nullptr;

    for(i = m_mounts.begin(), e = m_mounts.end(); i != e; i++)
    {
        // Yes, it's an instance compare, this is the best way to find it yet.
        if(i->backend == backend)
        {
            m_mounts.erase(i);
            break;
        }
    }

    return (i ? E_BIO_SUCCESS : E_BIO_ERR_ARGUMENT);
}

FsBackend* VirtualFileSystem::systemBackend() const
{
    return m_systemBackend;
}

const char* VirtualFileSystem::nativePathTranslation(const char* path, StringEditor& result) const
{
    if(isnil(path))
        return nullptr;

    const char* mount;
    FsBackend* fs = findBackend(path, &mount);

    if(fs && mount)
    {
        size_t len = strlen(mount);
        if(!len) // returned system backend, aka default branch
            return nullptr;

        path += len + 1;
        fs->nativePathTranslation(path, result);
    }
    else
    {
        return path;
    }

    return c_str(result);
}
}