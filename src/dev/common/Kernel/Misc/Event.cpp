#include <Engine.Base/Misc/Event.h>

#ifdef SvcCompilerMSVC
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

namespace rw
{
Event::Event(bool manualReset, bool initialState, const char* name)
    : m_eventHandle(CreateEventA(NIL, BOOL(manualReset), BOOL(initialState), name))
{}

Event::~Event()
{
    if(m_eventHandle)
    {
        CloseHandle(m_eventHandle);
        m_eventHandle = NIL;
    }
}

bool Event::set() const
{
    return SetEvent(m_eventHandle);
}

bool Event::reset() const
{
    return ResetEvent(m_eventHandle);
}

EventState Event::testState(u32 waitMs) const
{
    DWORD dwResult = WaitForSingleObject(HANDLE(m_eventHandle), DWORD(waitMs));
    if(dwResult == WAIT_OBJECT_0)
        return EventState::Set;
    if(dwResult == WAIT_TIMEOUT)
        return EventState::Reset;
    
    return EventState::Error;
}
}