#include <Engine.Base/Buffer/Stream/EndianInStream.h>

namespace rw
{
template<typename T>
inline void readEndian(Buffer* f, Endianness e, T& result)
{
    if(f->read(&result, sizeof(T)) >= E_BIO_SUCCESS)
    {
        switch(e)
        {
            case LittleEndian:
                result = toLittleEndian(tmp);
                break;
            case BigEndian:
                result = toBigEndian(tmp);
                break;
    
            default:
                break;
        }
    }
}

EndianInStream::EndianInStream(Buffer* f, Endianness endian)
    : m_buffer(f)
    , m_endian(endian)
{
    EASSERT(m_buffer, "No buffer provided for EndianInStream");
    EASSERT(m_buffer->canRead(), "Provided buffer is not readable");
}

EndianInStream::~EndianInStream()
{
    m_buffer = nullptr;
}

Endianness EndianInStream::getCurrentEndian() const
{
    return m_endian;
}

BIORETURN EndianInStream::read(void* buf, StreamSize size) const
{
    return m_buffer->read(buf, size);
}

// BIORETURN EndianInStream::write(const void*, StreamSize)
// {

// }

StreamSize EndianInStream::getAvailableBytes() const
{
    return m_buffer->getAvailableBytes();
}

StreamSize EndianInStream::getPosition() const
{
    return m_buffer->getPosition();
}

void EndianInStream::setPosition(StreamSize pos) const
{
    m_buffer->setPosition(pos);
}

EndianInStream& EndianInStream::operator>>(s8& result)
{
    return *this;
}

EndianInStream& EndianInStream::operator>>(u8& result)
{
    readEndian(m_buffer, m_endian, result);
    return *this;
}

EndianInStream& EndianInStream::operator>>(s16& result)
{
    readEndian(m_buffer, m_endian, result);
    return *this;
}

EndianInStream& EndianInStream::operator>>(u16& result)
{
    readEndian(m_buffer, m_endian, result);
    return *this;
}

EndianInStream& EndianInStream::operator>>(s32& result)
{
    readEndian(m_buffer, m_endian, result);
    return *this;
}

EndianInStream& EndianInStream::operator>>(u32& result)
{
    readEndian(m_buffer, m_endian, result);
    return *this;
}

EndianInStream& EndianInStream::operator>>(s64& result)
{
    readEndian(m_buffer, m_endian, result);
    return *this;
}

EndianInStream& EndianInStream::operator>>(u64& result)
{
    readEndian(m_buffer, m_endian, result);
    return *this;
}

EndianInStream& EndianInStream::operator>>(float& result)
{
    readEndian(m_buffer, m_endian, result);
    return *this;
}

EndianInStream& EndianInStream::operator>>(double& result)
{
    readEndian(m_buffer, m_endian, result);
    return *this;
}
}