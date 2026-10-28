#include <Engine.Storage/Svc/Depot.h>
#include <Engine.Core/diag/ExecutionContext.h>

namespace rw::esis
{
const char* streamTypeToString(StreamSpec::Type type)
{
    switch(type)
    {
        case StreamSpec::RELEASE: return "RELEASE";
        case StreamSpec::MAINLINE: return "MAINLINE";
        case StreamSpec::FEATURE: return "FEATURE";
        case StreamSpec::VIRTUAL: return "VIRTUAL";
        default: return "";
    }
}

StreamSpec::Type streamTypeToValue(const char* type)
{
    if(equalsIgnoreCase("release", type))
        return StreamSpec::RELEASE;
    if(equalsIgnoreCase("staging", type))
        return StreamSpec::MAINLINE;
    if(equalsIgnoreCase("feature", type))
        return StreamSpec::FEATURE;
    if(equalsIgnoreCase("virtual", type))
        return StreamSpec::VIRTUAL;

    return StreamSpec::MAXTYPES;
}

StreamSpec::Type MapAboveStream(StreamSpec::Type type)
{
    switch(type)
    {
        case StreamSpec::RELEASE: return StreamSpec::RELEASE;
        case StreamSpec::MAINLINE: return StreamSpec::RELEASE;
        case StreamSpec::FEATURE: return StreamSpec::MAINLINE;
        case StreamSpec::VIRTUAL: return StreamSpec::FEATURE;
    }
}

StreamSpec::Type MapBellowStream(StreamSpec::Type type)
{
    switch(type)
    {
        case StreamSpec::RELEASE: return StreamSpec::MAINLINE;
        case StreamSpec::MAINLINE: return StreamSpec::FEATURE;
        case StreamSpec::FEATURE: return StreamSpec::VIRTUAL;
        case StreamSpec::VIRTUAL: return StreamSpec::VIRTUAL;
    }
}

Domain::Domain(const char* short_name, const char* long_name, const char* root, StreamSpec::Type initial_stream)
    : m_storage(root)
{
    for(u32 i = 0; i < StreamSpec::MAXTYPES; i++)
        m_spec.streams[i] = nullptr;
    const char* domainPrefix = (new FormattedStringBuilder<128>("//%s", short_name))->c_str();
    
    StringBuilder<> dir;
    ExecutionContext::getVirtualFileSystem()->nativePathTranslation(root, dir);
    const char* absoluteDomainDir = dir.detach();

    m_backend = ExecutionContext::getVirtualFileSystem()->mount(domainPrefix, absoluteDomainDir);
    Init(short_name, long_name, initial_stream);
}

void Domain::Init(const char* short_name, const char* long_name, StreamSpec::Type type)
{
    m_spec.name = short_name;
    m_spec.long_name = long_name;
    
    Stream* initStream = reinterpret_cast<Stream*>(AttachStream(type, short_name));
    if(!initStream)
        return;

    ELOGF("%s - Created initial depot stream (%s)", short_name, streamTypeToString(type));
    m_spec.access_time = m_spec.modified_time = m_spec.creation_time = Time::unixNow();
}

Domain::~Domain()
{

}

Stream* Domain::SyncUp(StreamSpec::Type type, const char* source, const char* dest)
{
    Stream* downstream = SearchStream(type, source);
    __builtin_assert(downstream);

    Stream* upstream = downstream->GetAdjacentStream(StreamSpec::UP);

    // automatically create a target stream
    if(!upstream)
    {
        StreamSpec::Type above = MapAboveStream(downstream->GetType());
        upstream = AttachStream(above, dest);
        upstream->SetAdjacentStream(StreamSpec::DOWN, downstream);
    }

    ELOGF("%s - Synced up %s (%s) to %s (%s)", m_spec.name, downstream->GetName(), streamTypeToString(downstream->GetType()), upstream->GetName(), streamTypeToString(upstream->GetType()));
    // TODO: do actual syncing and locking
    return reinterpret_cast<Stream*>(upstream);
}

Stream* Domain::SyncDown(StreamSpec::Type type, const char* source, const char* dest)
{
    Stream* upstream = SearchStream(type, source);
    __builtin_assert(upstream);

    Stream* downstream = upstream->GetAdjacentStream(StreamSpec::DOWN);

    // automatically create a target stream
    if(!downstream)
    {
        StreamSpec::Type bellow = MapBellowStream(upstream->GetType());
        downstream = AttachStream(bellow, dest);
        downstream->SetAdjacentStream(StreamSpec::UP, upstream);
    }
    else
    {
        downstream = AttachStream(downstream->GetType(), dest);
    }

    ELOGF("%s - Synced down %s (%s) to %s (%s)", m_spec.name, upstream->GetName(), streamTypeToString(upstream->GetType()), downstream->GetName(), streamTypeToString(downstream->GetType()));
    // TODO: do actual syncing and locking
    return reinterpret_cast<Stream*>(downstream);
}

Stream* Domain::SearchStream(StreamSpec::Type type, const char* name)
{
    Stream* streamCsr = GetFirstStreamOf(type);
    if(!streamCsr)
        return nullptr;

    while(streamCsr)
    {
        if(equalsIgnoreCase(name, streamCsr->GetName()))
            return streamCsr;

        streamCsr = streamCsr->GetAdjacentStream(StreamSpec::RIGHT);
    }

    return nullptr;
}

Stream* Domain::AttachStream(StreamSpec::Type type, const char* name)
{
    typedef StreamSpec* stream_ref;
    typedef stream_ref* stream_iterator;
    
    Stream* stream_ = nullptr;
    if(!m_spec.streams[type])
    {
        stream_ = new Stream(name, type);
        m_spec.streams[type] = &stream_->GetInfo();
        ELOGF("%s - Attached stream %s (%s)", m_spec.name, name, streamTypeToString(type));
        return reinterpret_cast<Stream*>(m_spec.streams[type]);
    }
    
    stream_iterator insertPos = m_spec.streams + type;
    while((*insertPos)->neighbours[StreamSpec::RIGHT])
    {
        insertPos = (*insertPos)->neighbours + StreamSpec::RIGHT;
    }

    StreamSpec* stream = &(new Stream(name, type))->GetInfo();
    ELOGF("%s - Attached stream %s (%s)", m_spec.name, name, streamTypeToString(type));
    (*insertPos)->neighbours[StreamSpec::RIGHT] = stream;
    return reinterpret_cast<Stream*>(stream);
}

void Domain::DetachStream(StreamSpec::Type type, const char* name)
{

}
}