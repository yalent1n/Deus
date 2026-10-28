#include "../Buffer/Win32/Win32Buffer.h"
#include <Engine.Core/Backends/Win32FsBackend.h>
#include <Engine.Base/Vec.h>
#include <Engine.Base/String/Util.h>
#include <Engine.Base/Crypto/AES.h>

#include <winnetwk.h>
#pragma comment(lib, "Mpr.lib")

namespace rw
{
bool NtCreateDirectoryA(const char* _Directory)
{
    size_t Length = std::strlen(_Directory);
    if(Length <= 0)
    {
        return FALSE;
    }

    char Buffer[MAX_PATH] {0};

    size_t i = 0;
    for(;;)
    {
        const char c = _Directory[i];
        if(rw::issep(c))
        {
            std::memcpy(Buffer, _Directory, i);
            Buffer[i] = '\0';
            BOOL bCreated = CreateDirectoryA(Buffer, NIL);
            //ELOGF("NtCreateDirectoryA: %s", Buffer);
            if(!bCreated && (GetLastError() != ERROR_ALREADY_EXISTS && GetLastError() != ERROR_ACCESS_DENIED))
            {
                return FALSE;
            }
        }

        if(!c)
            return TRUE;

        i++;
    }

    return TRUE;
}

class Win32FileIterator : public FileIterator
{
public:
    HANDLE hFindFile;
    WIN32_FIND_DATAA findData;

    Win32FileIterator()
        : hFindFile(INVALID_HANDLE_VALUE)
    {}

    Win32FileIterator(const char* searchPath)
    {
        hFindFile = FindFirstFileA(searchPath, &findData);
        EASSERT(hFindFile != INVALID_HANDLE_VALUE, "FindFirstFileA failed");
        
        if(findData.cFileName[0] == '.' && (findData.cFileName[1] == '.' || findData.cFileName[1] == '\0'))
        {
            (void) FindNextFileA(hFindFile, &findData);
        }
    }

    ~Win32FileIterator()
    {
        if(hFindFile != INVALID_HANDLE_VALUE)
        {
            FindClose(hFindFile);
            hFindFile = INVALID_HANDLE_VALUE;
        }
    }

    bool getNextFile(char* nameBuffer, size_t nameMax, FileInfo* outInfo) override
    {
        if(!FindNextFileA(hFindFile, &findData))
            return false;

        LARGE_INTEGER dataSize;
        dataSize.LowPart = findData.nFileSizeLow;
        dataSize.HighPart = findData.nFileSizeHigh;
        outInfo->dataSize = size_t(dataSize.QuadPart);

        LARGE_INTEGER lastModifiedTime;
        lastModifiedTime.LowPart = findData.ftLastWriteTime.dwLowDateTime;
        lastModifiedTime.HighPart = findData.ftLastWriteTime.dwHighDateTime;
        outInfo->lastModifiedTime = lastModifiedTime.QuadPart;

        outInfo->isDirectory = !!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
        outInfo->isReadOnly = !!(findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY);

        size_t fileNameLen = strlen(findData.cFileName);
        memcpy_s(nameBuffer, nameMax, findData.cFileName, fileNameLen);
        nameBuffer[fileNameLen] = '\0';
        return true;
    }    
};

WindowsFS::WindowsFS()
{}

WindowsFS::WindowsFS(StringFn_t&& dirFn)
    : FsBackend(std::move(dirFn))
{
    const char* dir = directory();
    if(!isnil(dir) && !issep(dir[0]))
    {
        EASSERTF(NtCreateDirectoryA(dir), "Failed to create directory (%08x) : %s", GetLastError(), dir);
    }
}

WindowsFS::WindowsFS(const char* dirAbs)
    : FsBackend(dirAbs)
{
    const char* dir = directory();
    if(!isnil(dir) && !issep(dir[0]))
    {
        EASSERTF(NtCreateDirectoryA(dir), "Failed to create directory (%08x) : %s", GetLastError(), dir);
    }
}

bool WindowsFS::authenticate(const char* domain, const char* name, const char* password)
{
    PathStringBuilder tmp;
    tmp << directory();

    StringBuilder<> lpUserName;
    lpUserName << domain << '\\' << name;

    NETRESOURCEA nResource = {};
    nResource.dwType = RESOURCETYPE_DISK;
    nResource.lpLocalName = NULL;
    nResource.lpRemoteName = tmp.data(); // NOTE: assumes THIS backend is a network share drive
    nResource.lpProvider = NULL;

    DWORD dwResult = WNetAddConnection2A(&nResource, password, c_str(lpUserName), 0);
    return dwResult == NO_ERROR;
}

Buffer* WindowsFS::touch(const char* path, BIOACCESS access)
{
    access = BIOACCESS(access | (E_BIO_SEEK|E_BIO_FLUSH));

    PathStringBuilder nativePath;
    nativePathTranslation(path, nativePath);
    
    {
        const char* cursor = nativePath.data() + nativePath.size() - 1;
        while(!issep(*cursor))
            cursor--;

        *const_cast<char*>(cursor) = '\0';
        
        EASSERTF(NtCreateDirectoryA(nativePath.c_str()), "Failed to create directory for file (%08x) : %s", GetLastError(), cursor + 1);
        fixPath(const_cast<char*>(cursor), 1);
    }

    return new Win32Buffer(BIOACCESS(access), nativePath.c_str());
}

Buffer* WindowsFS::openFile(const char* path, bool check)
{
    u16 access = E_BIO_READ | E_BIO_SHARE_READ;
    return touch(path, BIOACCESS(access));
}

Buffer* WindowsFS::createFile(const char* path, bool append, bool read)
{
    u16 access = E_BIO_WRITE | E_BIO_SHARE_READ | __maybe(read, E_BIO_READ) | __maybe(append, E_BIO_APPEND);
    return touch(path, BIOACCESS(access));
}

SharedBuffer* WindowsFS::shareFile(const char* path, bool check)
{
    if(isnil(path))
        return NIL;

    PathStringBuilder nativePath;
    nativePathTranslation(path, nativePath);
    if(check)
    {
        DWORD dwAttributes = GetFileAttributesA(nativePath.c_str());
        if(dwAttributes == INVALID_FILE_ATTRIBUTES)
            return NIL;
    }

    return new MappedSharedBuffer(nativePath.c_str(), true);
}

bool WindowsFS::deleteFile(const char* path) const
{
    PathStringBuilder nativePath;
    nativePathTranslation(path, nativePath);    
    return DeleteFileA(nativePath.c_str());
}

bool WindowsFS::copyFile(const char* src, const char* dst)
{
    PathStringBuilder nativeSrcPath;
    nativePathTranslation(src, nativeSrcPath);

    PathStringBuilder nativeDstPath;
    nativePathTranslation(dst, nativeDstPath);

    return CopyFileA(c_str(nativeSrcPath), c_str(nativeDstPath), FALSE);
}

bool WindowsFS::moveFile(const char* src, const char* dst)
{
    PathStringBuilder nativeSrcPath;
    nativePathTranslation(src, nativeSrcPath);

    PathStringBuilder nativeDstPath;
    nativePathTranslation(dst, nativeDstPath);

    return MoveFileA(c_str(nativeSrcPath), c_str(nativeDstPath));
}

bool WindowsFS::sealFile(const char* dst)
{
    ScopedRef<Buffer> targetFile = createFile(dst, false);
    if(targetFile.isNull())
        return false;

    char tag[AesTagSize];

    size_t dataSize = targetFile->getAvailableBytes();
    
    Vec<u8> data;
    data.reserve(dataSize);

    if(targetFile->read(data.begin(), dataSize) < E_BIO_SUCCESS)
        return false;

    Vec<u8> encryptedData;
    encryptedData.reserve(dataSize);

    if(!AesEncrypt(AesKeySize256, AesCipherGCM, s_win32SealingKey, s_win32SealingKey, data.begin(), dataSize, encryptedData.begin(), encryptedData.capacity(), tag))
        return false;

    targetFile->setPosition(0);
    targetFile->write(encryptedData.begin(), dataSize);
    targetFile->write(tag, AesTagSize);
    targetFile->flush();
    return true;
}

bool WindowsFS::unsealFile(const char* dst)
{
    ScopedRef<Buffer> targetFile = createFile(dst, false);
    if(targetFile.isNull())
        return false;

    size_t dataSize = targetFile->getAvailableBytes();
    if(dataSize <= AesTagSize)
        return false;

    dataSize -= AesTagSize;

    Vec<u8> data;
    data.reserve(dataSize);

    if(targetFile->read(data.begin(), dataSize) < E_BIO_SUCCESS)
        return false;

    u8 tag[AesTagSize];
    if(targetFile->read(tag, AesTagSize) < E_BIO_SUCCESS)
        return false;

    Vec<u8> encryptedData;
    encryptedData.reserve(dataSize);

    if(!AesDecrypt(AesKeySize256, AesCipherGCM, s_win32SealingKey, s_win32SealingKey, data.begin(), dataSize, encryptedData.begin(), dataSize, tag))
        return false;

    targetFile->setPosition(0);
    targetFile->write(encryptedData.begin(), dataSize);
    targetFile->flush();
    return true;
}

bool WindowsFS::fileExists(const char* path) const
{        
    PathStringBuilder nativePath;
    nativePathTranslation(path, nativePath);
    return GetFileAttributesA(nativePath.c_str()) != INVALID_FILE_ATTRIBUTES;
}

bool WindowsFS::createDirectory(const char* path) const
{
    PathStringBuilder nativeDir;
    nativePathTranslation(path, nativeDir);
    
    BOOL bResult = NtCreateDirectoryA(nativeDir.c_str());
    return bResult;
}

bool WindowsFS::deleteDirectory(const char* path) const
{
    // const char* absolute = nativePathTranslation(path);
    // return DeleteFi
    return false;
}

FileIterator* WindowsFS::getFileIterator(const char* path) const
{
    PathStringBuilder filter;
    nativePathTranslation(path, filter);
    return new Win32FileIterator(filter.c_str());
}
}