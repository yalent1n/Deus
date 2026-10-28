#include <Engine.Storage/Svc/Stream.h>

namespace rw::esis
{
Stream::Stream(const char* name, StreamSpec::Type type)
{
    Init(name, type);
}

Stream::~Stream()
{}

void Stream::Init(const char* name, StreamSpec::Type type)
{
    m_spec.name = name;
    m_spec.type = type;

    for(u32 i = 0; i < StreamSpec::MAXDIRECTIONS; ++i)
        m_spec.neighbours[i] = nullptr;
}

void Stream::SetAdjacentStream(StreamSpec::Direction direction, Stream* stream)
{ 
    m_spec.neighbours[direction] = &stream->GetInfo();
    switch(direction)
    {
        case StreamSpec::UP:
        {
            stream->m_spec.neighbours[StreamSpec::DOWN] = &m_spec;
            break;
        }
        case StreamSpec::DOWN:
        {
            stream->m_spec.neighbours[StreamSpec::UP] = &m_spec;
            break;
        }
        case StreamSpec::LEFT:
        {
            stream->m_spec.neighbours[StreamSpec::RIGHT] = &m_spec;
            break;
        }
        case StreamSpec::RIGHT:
        {
            stream->m_spec.neighbours[StreamSpec::LEFT] = &m_spec;
            break;
        }
        default:
            break;
    }
}
}