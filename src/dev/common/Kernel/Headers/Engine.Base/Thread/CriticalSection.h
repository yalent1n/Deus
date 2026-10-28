#pragma once
#include "../Bundle.h"

namespace rw
{
#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunused-private-field"
#endif

class CriticalSection
{
public:
    FB_BASE_API CriticalSection(u32 SpinCount = 50);
    FB_BASE_API ~CriticalSection();

    FB_BASE_API void enter();
    FB_BASE_API bool tryEnter();
    FB_BASE_API void leave();
    FB_BASE_API bool isLocked();

private:
    // copied from the winnt.h. only difference is that this doesn't have the debug info pointer
	void* _;
    int m_LockCount;
    int m_RecursionCount;
    void* m_OwningThread;
    void* m_LockSemaphore;
    u64 m_SpinCount;
};

class CriticalSectionScope
{
public:
    CriticalSectionScope(CriticalSection& cs)
        : m_cs(&cs)
    {
        m_cs->enter();
    }

    ~CriticalSectionScope()
    {
        if(m_cs)
        {
            m_cs->leave();
            m_cs = 0;
        }
    }

private:
    CriticalSection* m_cs;
};

#ifdef __clang__
# pragma clang diagnostic pop
#endif
}