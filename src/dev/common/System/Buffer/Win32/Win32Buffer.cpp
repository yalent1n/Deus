#include "Win32Buffer.h"

namespace rw
{
Win32Buffer::Win32Buffer()
    : mHandle(INVALID_HANDLE_VALUE)
{}

Win32Buffer::Win32Buffer(BIOACCESS access, const char* path)
    : mHandle(INVALID_HANDLE_VALUE)
{
    m_access = access;
    (void) Open(path); // TODO: Create assert macros to catch runtime errors.
}

Win32Buffer::~Win32Buffer()
{
    if(m_access & E_BIO_FLUSH)
    {
        flush();
    }

    Close();
}

bool Win32Buffer::IsOpened() const
{
    return mHandle != INVALID_HANDLE_VALUE;
}

bool Win32Buffer::Open(const char* path)
{
    DWORD dwDesiredAccess = 0;
    if(m_access & E_BIO_READ)
        dwDesiredAccess |= GENERIC_READ;
    if(m_access & E_BIO_WRITE)
        dwDesiredAccess |= GENERIC_WRITE;

    DWORD dwShareMode = 0;
    if(m_access & E_BIO_SHARE_WRITE)
        dwShareMode |= FILE_SHARE_WRITE;
    if(m_access & E_BIO_SHARE_READ)
        dwShareMode |= FILE_SHARE_READ;

    DWORD dwCreationDisposition = (m_access & E_BIO_ALWAYS_CREATE) ? CREATE_ALWAYS : OPEN_ALWAYS;    
    DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
    if(m_access & E_BIO_ASYNC)
        dwFlagsAndAttributes |= FILE_FLAG_OVERLAPPED;

    HANDLE hFile = CreateFileA(path, dwDesiredAccess, dwShareMode, NIL, dwCreationDisposition, dwFlagsAndAttributes, NIL);
    if(hFile == INVALID_HANDLE_VALUE)
        return false;

    // let us manually handle where the file position is on file handle creation
    mHandle = hFile;
    seek(0, (m_access & E_BIO_APPEND ? E_BIO_SEEK_END : E_BIO_SEEK_START));
    return true;
}

bool Win32Buffer::Close()
{
    if(mHandle != INVALID_HANDLE_VALUE)
    {
        if(!CloseHandle(mHandle))
            return false;

        mHandle = INVALID_HANDLE_VALUE;
    }

    return true;
}

void Win32Buffer::flush()
{
    if(IsOpened() && (m_access & E_BIO_WRITE))
    {
        if(!SetEndOfFile(mHandle))
            return;
        (void) FlushFileBuffers(mHandle);
    }
}

void Win32Buffer::seek(size_t _Amount, BIOSEEKDIR _Direction)
{
    if(IsOpened() && (m_access & E_BIO_SEEK))
    {
        LARGE_INTEGER lDistanceToMove;
        lDistanceToMove.QuadPart = _Amount;
        SetFilePointerEx(mHandle, lDistanceToMove, NIL, DWORD(_Direction));
    }
}

s64 Win32Buffer::getAvailableBytes() const
{
    LARGE_INTEGER fileSize;
    GetFileSizeEx(mHandle, &fileSize);
    return fileSize.QuadPart;
}

s64 Win32Buffer::getPosition() const
{
    if(IsOpened())
    {
        LARGE_INTEGER lDistanceToMove;
        lDistanceToMove.QuadPart = 0;

        LARGE_INTEGER lNewFilePointer;
        SetFilePointerEx(mHandle, lDistanceToMove, &lNewFilePointer, FILE_CURRENT);
        return s64(lNewFilePointer.QuadPart);
    }

    return 0;
}

void Win32Buffer::setPosition(s64 pos)
{
    if(IsOpened() && (m_access & E_BIO_SEEK))
    {
        LARGE_INTEGER lDistanceToMove;
        lDistanceToMove.QuadPart = pos;
        SetFilePointerEx(mHandle, lDistanceToMove, NIL, FILE_BEGIN);
    }
}

BIORETURN Win32Buffer::write(const void* _Source, size_t _Length)
{
    if(!IsOpened())
        return E_BIO_ERR_NO_ACCESS;

    if(!_Source)
        return E_BIO_ERR_NO_IN_AVAIL;

    if(!_Length)
        return E_BIO_ERR_ARGUMENT;

    if(!(m_access & E_BIO_WRITE))
        return E_BIO_ERR_BIOACCESS;

    DWORD dwNumberOfBytesWritten = 0;
    BOOL Ret = WriteFile(mHandle, LPCVOID(_Source), DWORD(_Length), &dwNumberOfBytesWritten, NIL);
    return Ret ? BIORETURN(dwNumberOfBytesWritten) : E_BIO_ERR_OS;
}

BIORETURN Win32Buffer::writeAsync(const void* p, size_t len)
{
    return E_BIO_ERR_NO_IMPL;
}

BIORETURN Win32Buffer::read(void* _Target, size_t _Length)
{
    if(!IsOpened())
        return E_BIO_ERR_NO_ACCESS;

    if(!_Target)
        return E_BIO_ERR_NO_OUT_AVAIL;

    if(!_Length)
        return E_BIO_ERR_ARGUMENT;

    if(!(m_access & E_BIO_READ))
        return E_BIO_ERR_BIOACCESS;

    DWORD dwNumberOfBytesRead = 0;
    BOOL Ret = ReadFile(mHandle, LPVOID(_Target), DWORD(_Length), &dwNumberOfBytesRead, NIL);
    return Ret ? BIORETURN(dwNumberOfBytesRead) : E_BIO_ERR_OS;
}

BIORETURN Win32Buffer::readAsync(void* p, size_t len)
{
    return E_BIO_ERR_NO_IMPL;
}
}