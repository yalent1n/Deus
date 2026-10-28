#pragma once
#include "../Buffer.h"

namespace rw
{
class EndianInStream
{
public:
    FB_BASE_API  EndianInStream(Buffer* f, Endianness endian);
    FB_BASE_API ~EndianInStream();

    FB_BASE_API Endianness getCurrentEndian() const;

    // NOTE: This doesn't affect stream endianness!
    FB_BASE_API BIORETURN read(void*, StreamSize) const;
    
    // NOTE: This doesn't affect stream endianness!
    // FB_BASE_API BIORETURN write(const void*, StreamSize) const;

    FB_BASE_API StreamSize getAvailableBytes() const;

    FB_BASE_API StreamSize getPosition() const;
    FB_BASE_API void setPosition(StreamSize pos) const;

    FB_BASE_API EndianInStream& operator>>(s8& result);
    FB_BASE_API EndianInStream& operator>>(u8& result);
    FB_BASE_API EndianInStream& operator>>(s16& result);
    FB_BASE_API EndianInStream& operator>>(u16& result);
    FB_BASE_API EndianInStream& operator>>(s32& result);
    FB_BASE_API EndianInStream& operator>>(u32& result);
    FB_BASE_API EndianInStream& operator>>(s64& result);
    FB_BASE_API EndianInStream& operator>>(u64& result);
    FB_BASE_API EndianInStream& operator>>(float& result);
    FB_BASE_API EndianInStream& operator>>(double& result);

private:
    Buffer* m_buffer;
    Endianness m_endian;
};
}