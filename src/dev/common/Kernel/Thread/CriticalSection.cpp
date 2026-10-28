#include <Engine.Base/Thread/CriticalSection.h>

#ifdef SvcCompilerMSVC
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace rw
{
CriticalSection::CriticalSection(u32 SpinCount)
	: _(0)
	, m_LockCount(0)
	, m_RecursionCount(0)
	, m_OwningThread(0)
	, m_LockSemaphore(0)
	, m_SpinCount(0)
{
	EASSERT(InitializeCriticalSectionAndSpinCount((LPCRITICAL_SECTION)this, SpinCount), "Failed to initialize critical section.");
}

CriticalSection::~CriticalSection()
{
	DeleteCriticalSection((LPCRITICAL_SECTION)this);
}

void CriticalSection::enter()
{
	EnterCriticalSection((LPCRITICAL_SECTION)this);
}

bool CriticalSection::tryEnter()
{
	return TryEnterCriticalSection((LPCRITICAL_SECTION)this);
}

void CriticalSection::leave()
{
	LeaveCriticalSection((LPCRITICAL_SECTION)this);
}

bool CriticalSection::isLocked()
{
	return m_LockCount > 0;
}
}