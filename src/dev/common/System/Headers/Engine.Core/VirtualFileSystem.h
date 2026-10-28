#pragma once
#include "FsBackend.h"
#include <Engine.Base/Vec.h>

namespace rw
{
// In the current directory setup these variables corresponds to these paths:
//      local @ <executable directory>
//      working @ <working directory>
//      root @ G:
//      workspace @ g:/soccer
//      data @ g:/soccer/data
//      framework @ g:/soccer/framework
//      code @ g:/soccer/framework/code
//      bin @ g:/soccer/framework/bin

// Holds the default interface for file and directory access
// The base directory is the working directory (can nominate using native_root)
class VirtualFileSystem
{
public:
    struct cache_item
    {
        const char*     id;
        FsBackend*      backend;
    };

    typedef Vec<cache_item> Cache;

    CORE_API VirtualFileSystem();
    CORE_API ~VirtualFileSystem();

    // ----
    // File functions

    // See @ref FsBackend::touch()
    CORE_API Buffer*             touch(const char*, BIOACCESS = E_BIO_ALWAYS_CREATE);

    // See @ref FsBackend::openFile()
    CORE_API Buffer*             openFile(const char*, bool check = true);
    
    // See @ref FsBackend::createFile()
    CORE_API Buffer*             createFile(const char*, bool append = false, bool read = true);

    // See @ref FsBackend::shareFile()
    CORE_API SharedBuffer*       shareFile(const char*, bool check = true);

    // See @ref FsBackend::deleteFile()
    CORE_API bool                deleteFile(const char*);
    
    // See @ref FsBackend::copyFile()
    CORE_API bool                copyFile(const char*, const char*);
    
    // See @ref FsBackend::moveFile()
    CORE_API bool                moveFile(const char*, const char*);

    // See @ref FsBackend::sealFile()
    CORE_API bool                sealFile(const char*);
    
    // See @ref FsBackend::unsealFile()
    CORE_API bool                unsealFile(const char*);

    // See @ref FsBackend::fileExists()
    CORE_API bool                fileExists(const char*);

    // ----
    // Directory functions

    // Creates a directory.
    CORE_API bool createDirectory(const char*) const;

    // Deletes a directory and it's contents.
    CORE_API bool deleteDirectory(const char*) const;

    // Returns the file iterator for the directory.
    CORE_API FileIterator* getFileIterator(const char* searchPath) const;

    // Traverses the files and directories by the filter.
    CORE_API void traverseFiles(const char* filter, TraverseFn_t fn, bool excludeDirs = true);

    // ----
    // Mounting-related functions

    // Register a backend into the cache. This function will return the existing one if found.
    // To replace the existing one, simply close that.
    CORE_API FsBackend*      registerBackend(FsBackend*, const char*);

    // Mounts a directory (won't create if there is already one instance with the same identifier).
    CORE_API FsBackend*      mount(const char*, StringFn_t&& dir);
    
    // Mounts a directory (won't create if there is already one instance with the same identifier).
    CORE_API FsBackend*      mount(const char*, const char* dir);
    
    // Open a cached view to a directory and returns nullptr if wasn't found.
    CORE_API FsBackend*      findBackend(const char*, const char** mountResult = nullptr) const;

    // Closes the view for the instance.
    CORE_API BIORETURN       unmount(FsBackend*);

    // Returns the IO backend for the system (only absolute path).
    CORE_API FsBackend*      systemBackend() const;

    // Returns a translated path from virtual to system native.
    CORE_API const char*     nativePathTranslation(const char* path, StringEditor& result) const;

private:
    Cache m_mounts;
    FsBackend* m_systemBackend;
};
}