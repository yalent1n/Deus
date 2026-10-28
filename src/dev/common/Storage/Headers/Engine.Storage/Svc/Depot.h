#pragma once
#include "Stream.h"
#include "Storage.h"

namespace rw::esis
{
FB_STORAGE_API const char* streamTypeToString(StreamSpec::Type type);
FB_STORAGE_API StreamSpec::Type streamTypeToValue(const char* type);

class Domain
{
public:
    FB_STORAGE_API Domain(const char* short_name, const char* long_name, const char* root, StreamSpec::Type initial_stream = StreamSpec::MAINLINE);
    FB_STORAGE_API ~Domain();

    const DomainSpec& GetInfo() const
        { return m_spec; }
    
    DomainSpec& GetInfo()
        { return m_spec; }
    
    const Storage& GetStorage() const
        { return m_storage; }
    
    Storage& GetStorage()
        { return m_storage; }
    
    const FsBackend& GetBackend() const
        { return *m_backend; }
    
    FsBackend& GetBackend()
        { return *m_backend; }

    const Stream* GetFirstStreamOf(StreamSpec::Type type) const
        { return reinterpret_cast<Stream*>(m_spec.streams[type]); }
    
    Stream* GetFirstStreamOf(StreamSpec::Type type)
        { return reinterpret_cast<Stream*>(m_spec.streams[type]); }

    FB_STORAGE_API void Init(const char* short_name, const char* long_name, StreamSpec::Type type = StreamSpec::MAINLINE);

    FB_STORAGE_API Stream* SyncUp(StreamSpec::Type type, const char* source, const char* dest);
    FB_STORAGE_API Stream* SyncDown(StreamSpec::Type type, const char* source, const char* dest);

    FB_STORAGE_API Stream* SearchStream(StreamSpec::Type type, const char* name);
    FB_STORAGE_API Stream* AttachStream(StreamSpec::Type type, const char* name);
    FB_STORAGE_API void DetachStream(StreamSpec::Type type, const char* name);

private:
    DomainSpec m_spec;
    Storage m_storage;
    ScopedRef<FsBackend> m_backend;
};
}