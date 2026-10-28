#include <Engine.Base/Buffer/Buffer.h>
#include <Engine.Base/String/String.h>

namespace rw
{
Buffer::Buffer()
    : m_mimeType(Mime_OctetStream)
{}

Buffer::~Buffer()
{
    if(m_access & E_BIO_FLUSH)
    {
        flush();
    }
}

bool Buffer::canRead() const
{
    return !!(m_access & E_BIO_READ);
}

bool Buffer::canWrite() const
{
    return !!(m_access & E_BIO_WRITE);
}

bool Buffer::canSeek() const
{
    return !!(m_access & E_BIO_SEEK);
}

bool Buffer::canFlush() const
{
    return !!(m_access & E_BIO_FLUSH);
}

MIME Buffer::mimeType() const
{
    return m_mimeType;
}

void Buffer::setMimeType(MIME m)
{
    m_mimeType = m;
}

const u8* Buffer::bytes() const
{
    return nullptr; // nop
}

const u8* Buffer::detach() const
{
    return nullptr; // nop
}

SharedBuffer* Buffer::detachSharedBuffer() const
{
    return nullptr; // nop
}

void Buffer::flush()
{
    // nop
}

void Buffer::seek(size_t, BIOSEEKDIR)
{
    // nop
}

s64 Buffer::getAvailableBytes() const
{
    return 0; // nop
}

s64 Buffer::getPosition() const
{
    return 0; // nop
}

void Buffer::setPosition(s64 pos)
{
    // nop
}

BIORETURN Buffer::copyTo(Buffer& _Target, s64 _Amount)
{    
    size_t Len = _Amount;
    if(!Len)
    {
        return E_BIO_ERR_ARGUMENT;
    }
    
    if((getPosition() + _Amount) > getAvailableBytes())
    {
        Len = (size_t)(getAvailableBytes() - getPosition());
    }

    const u8* Src = bytes();
    if(!Src)
    {
        return E_BIO_ERR_NO_IN_AVAIL;
    }

    BIORETURN Ret = _Target.write(Src + getPosition(), Len);
    if(Ret >= E_BIO_SUCCESS)
    {
        seek(Len);
    }

    return Ret;
}

BIORETURN Buffer::write(const void* p, size_t len)
{
    if(!(m_access & E_BIO_WRITE))
        return E_BIO_ERR_BIOACCESS;

    return E_BIO_ERR_NO_IMPL;
}

BIORETURN Buffer::writeAsync(const void* p, size_t len)
{
    if(!(m_access & (E_BIO_WRITE | E_BIO_ASYNC)))
        return E_BIO_ERR_BIOACCESS;

    return E_BIO_ERR_NO_IMPL;
}

BIORETURN Buffer::read(void* p, size_t len)
{
    if(!(m_access & E_BIO_READ))
        return E_BIO_ERR_BIOACCESS;

    return E_BIO_ERR_NO_IMPL;
}

BIORETURN Buffer::readAsync(void* p, size_t len)
{
    if(!(m_access & (E_BIO_READ | E_BIO_ASYNC)))
        return E_BIO_ERR_BIOACCESS;

    return E_BIO_ERR_NO_IMPL;
}

s8 Buffer::readInt8()
{
    s8 Num = 0;
    (void) read(&Num, sizeof(Num));
    return Num;
}

u8 Buffer::readUint8()
{
    u8 Num = 0;
    (void) read(&Num, sizeof(Num));
    return Num;
}

s16 Buffer::readInt16()
{
    s16 Num = 0;
    (void) read(&Num, sizeof(Num));
    return Num;
}

u16 Buffer::readUint16()
{
    u16 Num = 0;
    (void) read(&Num, sizeof(Num));
    return Num;
}

s32 Buffer::readInt()
{
    s32 Num = 0;
    (void) read(&Num, sizeof(Num));
    return Num;
}

u32 Buffer::readUint()
{
    u32 Num = 0;
    (void) read(&Num, sizeof(Num));
    return Num;
}

s64 Buffer::readInt64()
{
    s64 Num = 0;
    (void) read(&Num, sizeof(Num));
    return Num;
}

u64 Buffer::readUint64()
{
    u64 Num = 0;
    (void) read(&Num, sizeof(Num));
    return Num;
}

f32 Buffer::readFloat()
{
    f32 Num = 0;
    (void) read(&Num, sizeof(Num));
    return Num;
}

f64 Buffer::readDouble()
{
    f64 Num = 0;
    (void) read(&Num, sizeof(Num));
    return Num;
}

const char* Buffer::decodeCString()
{
    StringBuilder<16> String;

    char c;
    for(;;)
    {
        if(read(&c, sizeof(c)) < E_BIO_SUCCESS)
            return NIL;

        if(!c)
            return String.detach();

        String.appendChar(c);
    }

    return NIL;
}

const wchar_t* Buffer::decodeWString()
{
    WideStringBuilder<16> String;

    wchar_t c;
    for(;;)
    {
        if(read(&c, sizeof(c)) < E_BIO_SUCCESS)
            return NIL;

        if(!c)
            return String.detach();

        String.appendChar(c);
    }

    return NIL;
}

u32 Buffer::decodeUint()
{
    u64 result = 0;
    int shift = 0;
    u8 byte;
    do {
        byte = readUint8();
        result |= (u32)(byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);

    return result;
}

u64 Buffer::decodeUint64()
{
    u64 result = 0;
    int shift = 0;
    u8 byte;
    do {
        byte = readUint8();
        result |= (u64)(byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);

    return result;
}

s32 Buffer::decodeInt()
{
    u32 Num = decodeUint();
    return (Num >> 1) ^ -(s32)(Num & 1);
}

s64 Buffer::decodeInt64()
{
    u64 Num = decodeUint64();
    return (Num >> 1) ^ -(s64)(Num & 1);
}

BIORETURN Buffer::write(s8 _Num)
{
    return write(&_Num, sizeof(_Num));
}

BIORETURN Buffer::write(u8 _Num)
{
    return write(&_Num, sizeof(_Num));
}

BIORETURN Buffer::write(s16 _Num)
{
    return write(&_Num, sizeof(_Num));
}

BIORETURN Buffer::write(u16 _Num)
{
    return write(&_Num, sizeof(_Num));
}

BIORETURN Buffer::write(s32 _Num)
{
    return write(&_Num, sizeof(_Num));
}

BIORETURN Buffer::write(u32 _Num)
{
    return write(&_Num, sizeof(_Num));
}

BIORETURN Buffer::write(s64 _Num)
{
    return write(&_Num, sizeof(_Num));
}

BIORETURN Buffer::write(u64 _Num)
{
    return write(&_Num, sizeof(_Num));
}

BIORETURN Buffer::write(f32 _Num)
{
    return write(&_Num, sizeof(_Num));
}

BIORETURN Buffer::write(f64 _Num)
{
    return write(&_Num, sizeof(_Num));
}

BIORETURN Buffer::encodeString(const char* _String)
{
    if(_String == NIL)
        return E_BIO_ERR_ARGUMENT;

    const char* Begin = _String;

    while(*_String)
    {
        BIORETURN Ret = write(_String, 1);
        if(Ret < E_BIO_SUCCESS)
            return Ret;

        _String++;
    }

    write(char(0));
    return BIORETURN(_String - Begin);
}

BIORETURN Buffer::encodeWString(const wchar_t* _String)
{
    if(_String == NIL)
        return E_BIO_ERR_ARGUMENT;

    const wchar_t* Begin = _String;

    while(*_String)
    {
        BIORETURN Ret = write(_String, 2);
        if(Ret < E_BIO_SUCCESS)
            return Ret;

        _String++;
    }

    write(u16(0));
    return BIORETURN(_String - Begin);
}

BIORETURN Buffer::encodeUint(u32 _Num)
{
    while(_Num >= 0x80) 
    {
        BIORETURN Ret = write(static_cast<u8>(_Num | 0x80));
        if(Ret < E_BIO_SUCCESS)
            return Ret;        
        _Num >>= 7;
    }

    return write(static_cast<u8>(_Num));
}

BIORETURN Buffer::encodeUint64(u64 _Num)
{
    while(_Num >= 0x80) 
    {
        BIORETURN Ret = write(static_cast<u8>(_Num | 0x80));
        if(Ret < E_BIO_SUCCESS)
            return Ret;        
        _Num >>= 7;
    }

    return write(static_cast<u8>(_Num));
}

BIORETURN Buffer::encodeInt(s32 _Num)
{
    u32 z = static_cast<u32>((_Num << 1) ^ (_Num >> 31));
    return encodeUint(z);
}

BIORETURN Buffer::encodeInt64(s64 _Num)
{
    u64 z = static_cast<u64>((_Num << 1) ^ (_Num >> 63));
    return encodeUint64(z);
}
}