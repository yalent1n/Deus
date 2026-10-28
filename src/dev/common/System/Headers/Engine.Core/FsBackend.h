#pragma once
#include "Bundle.h"
#include <Engine.Base/Buffer/Buffer.h>
#include <Engine.Base/Buffer/SharedBuffer.h>
#include <Engine.Base/String/String.h>
#include <functional>

namespace rw
{
struct FileInfo
{
    s64 lastModifiedTime;
    size_t dataSize;
    u8 isDirectory : 1;
    u8 isReadOnly : 1;
    u8 _reserved : 1;
    u8 userFlags : 5;
};

class FileIterator
{
public:
    CORE_API          FileIterator();
    CORE_API virtual ~FileIterator();

    nodefine bool getNextFile(char* nameBuffer, size_t nameMax, FileInfo* outInfo) = NIL;
};

typedef std::function<const char*()> StringFn_t;
typedef std::function<void(const char*, const FileInfo&)> TraverseFn_t;

class FsBackend
{
public:
    CORE_API FsBackend();
    CORE_API FsBackend(StringFn_t&&);
    CORE_API FsBackend(const char*);
    virtual ~FsBackend() = default;

    bool isDynamic() const { return m_isDynamic; }

    // ----
    // User functions
    nodefine bool authenticate(const char* domain, const char* name, const char* password) { return true; }

    // ----
    // File functions

    // Creates an input/output buffer from a file path.
    nodefine Buffer* touch(const char*, BIOACCESS = E_BIO_ALWAYS_CREATE) = NIL;

    // Opens a file for reading. File creation depends on the configuration.
    nodefine Buffer* openFile(const char*, bool check = true) = NIL;

    // Opens a file for reading and/or writing. File creation depends on the configuration.
    nodefine Buffer* createFile(const char*, bool append = false, bool read = true) = NIL;

    // Opens a file as a mapped object. File existancy is checked by the parameter.
    nodefine SharedBuffer* shareFile(const char*, bool check = true) = NIL;    

    // Deletes a file.
    nodefine bool deleteFile(const char*) const = NIL;

    // Copies a file from source path to destination path (can only do it locally, no network drives).
    nodefine bool copyFile(const char*, const char*) = NIL;
    
    // Moves a file from source path to destination path (can only do it locally, no network drives).
    nodefine bool moveFile(const char*, const char*) = NIL;

    // Encrypts a file using the backend's sealing key.
    nodefine bool sealFile(const char*) = NIL;
    
    // Encrypts a file using the backend's sealing key.
    nodefine bool unsealFile(const char*) = NIL;

    // Checks if a file exists.
    nodefine bool fileExists(const char*) const = NIL;

    // ----
    // Directory functions

    // Creates a directory.
    nodefine bool createDirectory(const char*) const = NIL;

    // Deletes a directory and it's contents.
    nodefine bool deleteDirectory(const char*) const = NIL;

    // Returns the file iterator for the directory.
    nodefine FileIterator* getFileIterator(const char*) const = NIL;

    // Traverse every file and directory by the filter (subdirectories too).
    CORE_API bool traverseFiles(const char* filter, TraverseFn_t fn, bool excludeDirs = true);

    // ----
    // Platform-independent functions

    // Translates the relative path to an absolute path.
    CORE_API void nativePathTranslation(const char*, StringEditor& result) const;

    // Returns the directory which we operate in.
    CORE_API const char* directory() const;

protected:
    StringFn_t m_dirFnPath;
    const char* m_dirAbsPath;
    bool m_isDynamic;
};
}