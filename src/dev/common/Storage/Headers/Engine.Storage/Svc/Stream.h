#pragma once
#include "esis.h"

namespace rw::esis
{
class Stream
{
public:
    FB_STORAGE_API Stream(const char* name, StreamSpec::Type type);
    FB_STORAGE_API ~Stream();

    const StreamSpec& GetInfo() const
        { return m_spec; }
    
    StreamSpec& GetInfo()
        { return m_spec; }

    StreamSpec::Type GetType() const
        { return m_spec.type; }

    const char* GetName() const
        { return m_spec.name; }
    
    Stream* GetAdjacentStream(StreamSpec::Direction direction) const
        { return reinterpret_cast<Stream*>(m_spec.neighbours[direction]); }

    FB_STORAGE_API void SetAdjacentStream(StreamSpec::Direction direction, Stream* stream);

    FB_STORAGE_API void Init(const char* name, StreamSpec::Type type);

private:
    StreamSpec m_spec;
};
}