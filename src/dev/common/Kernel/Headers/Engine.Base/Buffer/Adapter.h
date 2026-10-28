#pragma once
#include "Buffer.h"
#include "SharedBuffer.h"

namespace rw
{
class Adapter;
typedef Adapter AllocatingBuffer; // for syntax sugar.

class Adapter : public Buffer
{
public:
    // This serves as a null thing and as a memory block builder.
    // The reason is why this won't require a size, because it uses 
    // virtual memory (hence the parameter) to allocate new pages for building.
    FB_BASE_API Adapter(bool virt = true);
    FB_BASE_API Adapter(const void* rodata, size_t rolen, OwnershipPolicy ownership = kNoOwnership);

    FB_BASE_API const u8* bytes() const override;
    FB_BASE_API const u8* detach() const override;
    FB_BASE_API SharedBuffer* detachSharedBuffer() const override;
    FB_BASE_API const u8* marker() const;
    FB_BASE_API const size_t position() const;
    FB_BASE_API const size_t length() const;
    FB_BASE_API OwnershipPolicy ownershipPolicy() const;

    FB_BASE_API void* allocateWrite(size_t);
        
    FB_BASE_API void flush() override;
    FB_BASE_API void seek(size_t, BIOSEEKDIR = E_BIO_SEEK_CUR) override;

    FB_BASE_API s64 getAvailableBytes() const override;
    FB_BASE_API s64 getPosition() const override;
    FB_BASE_API void setPosition(s64 pos) override;

    FB_BASE_API BIORETURN read(void*, size_t) override;
    FB_BASE_API BIORETURN write(const void*, size_t) override;

    // Typed functions
    FB_BASE_API s8 readInt8() { return Buffer::readInt8(); }
    FB_BASE_API u8 readUint8() { return Buffer::readUint8(); }
    FB_BASE_API s16 readInt16() { return Buffer::readInt16(); }
    FB_BASE_API u16 readUint16() { return Buffer::readUint16(); }
    FB_BASE_API s32 readInt32() { return Buffer::readInt(); }
    FB_BASE_API u32 readUint32() { return Buffer::readUint(); }
    FB_BASE_API s64 readInt64() { return Buffer::readInt64(); }
    FB_BASE_API u64 readUint64() { return Buffer::readUint64(); }
    FB_BASE_API f32 readFloat() { return Buffer::readFloat(); }
    FB_BASE_API f64 readDouble() { return Buffer::readDouble(); }
    FB_BASE_API const char* decodeString() { return Buffer::decodeCString(); }
    FB_BASE_API const wchar_t* decodeWString() { return Buffer::decodeWString(); }
    FB_BASE_API u32 decodeUint() { return Buffer::decodeUint(); }
    FB_BASE_API u64 decodeUint64() { return Buffer::decodeUint64(); }
    FB_BASE_API s32 decodeInt() { return Buffer::decodeInt(); }
    FB_BASE_API s64 decodeInt64() { return Buffer::decodeInt64(); }
    
    FB_BASE_API BIORETURN write(s8 _Num) { return Buffer::write(_Num); }
    FB_BASE_API BIORETURN write(u8 _Num) { return Buffer::write(_Num); }
    FB_BASE_API BIORETURN write(s16 _Num) { return Buffer::write(_Num); }
    FB_BASE_API BIORETURN write(u16 _Num) { return Buffer::write(_Num); }
    FB_BASE_API BIORETURN write(s32 _Num) { return Buffer::write(_Num); }
    FB_BASE_API BIORETURN write(u32 _Num) { return Buffer::write(_Num); }
    FB_BASE_API BIORETURN write(s64 _Num) { return Buffer::write(_Num); }
    FB_BASE_API BIORETURN write(u64 _Num) { return Buffer::write(_Num); }
    FB_BASE_API BIORETURN write(f32 _Num) { return Buffer::write(_Num); }
    FB_BASE_API BIORETURN write(f64 _Num) { return Buffer::write(_Num); }
    FB_BASE_API BIORETURN encodeString(const char* _String) { return Buffer::encodeString(_String); }
    FB_BASE_API BIORETURN encodeWString(const wchar_t* _String) { return Buffer::encodeWString(_String); }
    FB_BASE_API BIORETURN encodeUint(u32 _Num) { return Buffer::encodeUint(_Num); }
    FB_BASE_API BIORETURN encodeUint64(u64 _Num) { return Buffer::encodeUint64(_Num); }
    FB_BASE_API BIORETURN encodeInt(s32 _Num) { return Buffer::encodeInt(_Num); }
    FB_BASE_API BIORETURN encodeInt64(s64 _Num) { return Buffer::encodeInt64(_Num); }

private:
    bool ensure(size_t);

private:
    u8* m_bytes;
    u8* m_end;
    size_t m_position;
    size_t m_length;
    OwnershipPolicy m_ownership;
    bool m_isDynamic;
};
}