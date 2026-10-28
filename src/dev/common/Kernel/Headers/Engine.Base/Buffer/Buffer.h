#pragma once
#include "SharedBuffer.h"

namespace rw
{
enum BIOACCESS : u16
{
    E_BIO_READ              = 1 << 1,
    E_BIO_WRITE             = 1 << 2,
    E_BIO_SEEK              = 1 << 3,
    E_BIO_FLUSH             = 1 << 4,

    // Extended capabilities
    E_BIO_ALWAYS_CREATE     = 1 << 5,
    E_BIO_APPEND            = 1 << 6,
    E_BIO_ASYNC             = 1 << 7,
    E_BIO_SHARE_WRITE       = 1 << 8,
    E_BIO_SHARE_READ        = 1 << 9
};

enum BIOSEEKDIR : u8
{
    E_BIO_SEEK_START = 0,
    E_BIO_SEEK_CUR = 1,
    E_BIO_SEEK_END = 2
};

enum BIORETURN : s32
{
    E_BIO_ERR = -9,
    E_BIO_ERR_ARGUMENT,
    E_BIO_ERR_OS,
    E_BIO_ERR_END_REACHED,
    E_BIO_ERR_NO_IN_AVAIL,
    E_BIO_ERR_NO_OUT_AVAIL,
    E_BIO_ERR_BIOACCESS,
    E_BIO_ERR_NO_IMPL,
    E_BIO_ERR_NO_ACCESS,
    
    // We don't really use it, it's just a syntactic sugar when checking return values.
    E_BIO_SUCCESS
};

class Buffer
{
public:
    FB_BASE_API Buffer();

    // Additional close functionalities can be managed by the capabilities.
    FB_BASE_API nodefine ~Buffer();

    // Capability functions
    FB_BASE_API bool canRead() const;
    FB_BASE_API bool canWrite() const;
    FB_BASE_API bool canSeek() const;
    FB_BASE_API bool canFlush() const;

    // MIME
    FB_BASE_API MIME mimeType() const;
    FB_BASE_API void setMimeType(MIME);

    // Unified functions
    FB_BASE_API nodefine const u8* bytes() const;
    FB_BASE_API nodefine const u8* detach() const;
    FB_BASE_API nodefine SharedBuffer* detachSharedBuffer() const;
    FB_BASE_API nodefine void flush();
    FB_BASE_API nodefine void seek(size_t, BIOSEEKDIR = E_BIO_SEEK_CUR);
    FB_BASE_API nodefine s64 getAvailableBytes() const;
    FB_BASE_API nodefine s64 getPosition() const;
    FB_BASE_API nodefine void setPosition(s64 pos);
    FB_BASE_API BIORETURN copyTo(Buffer&, s64);

    // Custom functions
    FB_BASE_API nodefine BIORETURN write(const void* p, size_t len);
    FB_BASE_API nodefine BIORETURN writeAsync(const void* p, size_t len);
    FB_BASE_API nodefine BIORETURN read(void* p, size_t len);
    FB_BASE_API nodefine BIORETURN readAsync(void* p, size_t len);

    // Typed functions
    FB_BASE_API s8 readInt8();
    FB_BASE_API u8 readUint8();
    FB_BASE_API s16 readInt16();
    FB_BASE_API u16 readUint16();
    FB_BASE_API s32 readInt();
    FB_BASE_API u32 readUint();
    FB_BASE_API s64 readInt64();
    FB_BASE_API u64 readUint64();
    FB_BASE_API f32 readFloat();
    FB_BASE_API f64 readDouble();

    FB_BASE_API const char* decodeCString();
    FB_BASE_API const wchar_t* decodeWString();
    
    FB_BASE_API u32 decodeUint(); // signed 32-bit varint
    FB_BASE_API u64 decodeUint64(); // unsigned 64-bit varint
    FB_BASE_API s32 decodeInt(); // signed 32-bit varint
    FB_BASE_API s64 decodeInt64(); // unsigned 64-bit varint
    
    FB_BASE_API BIORETURN write(s8 _Num);
    FB_BASE_API BIORETURN write(u8 _Num);
    FB_BASE_API BIORETURN write(s16 _Num);
    FB_BASE_API BIORETURN write(u16 _Num);
    FB_BASE_API BIORETURN write(s32 _Num);
    FB_BASE_API BIORETURN write(u32 _Num);
    FB_BASE_API BIORETURN write(s64 _Num);
    FB_BASE_API BIORETURN write(u64 _Num);
    FB_BASE_API BIORETURN write(f32 _Num);
    FB_BASE_API BIORETURN write(f64 _Num);

    FB_BASE_API BIORETURN encodeString(const char* _String);
    FB_BASE_API BIORETURN encodeWString(const wchar_t* _String);
    
    FB_BASE_API BIORETURN encodeUint(u32 _Num);
    FB_BASE_API BIORETURN encodeUint64(u64 _Num);
    FB_BASE_API BIORETURN encodeInt(s32 _Num);
    FB_BASE_API BIORETURN encodeInt64(s64 _Num);

protected:
    BIOACCESS m_access;
    MIME m_mimeType;
};
}