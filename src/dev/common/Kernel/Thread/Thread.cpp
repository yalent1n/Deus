#include <Engine.Base/Thread/Thread.h>

#ifdef SvcCompilerMSVC
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include <process.h>
#endif

namespace rw
{
	int _getpriority(Priority prio)
	{
		switch(prio)
		{
		case Priority::BelowNormal:
			return THREAD_PRIORITY_BELOW_NORMAL;
		case Priority::Normal:
			return THREAD_PRIORITY_NORMAL;
		case Priority::AboveNormal:
			return THREAD_PRIORITY_ABOVE_NORMAL;
		case Priority::High:
			return THREAD_PRIORITY_ABOVE_NORMAL + 1;
		case Priority::TimeCritical:
			return THREAD_PRIORITY_TIME_CRITICAL;
		}
	}
	
	Thread::Thread(const char* name, u32 stackSize, ThreadStart startFunc, void* userParam, int processor, Priority priority, ThreadAffinityMask affinity)
		: mThreadName(name)
		, mThreadId(nullptr)
		, mStatus(ThreadStatus::kStatusInactive)
	{
		ThreadParameters params;
		params.mnStackSize = stackSize;
		params.mnPriority = priority;
		params.mnProcessor = processor;
		params.mnAffinityMask = affinity;
		Begin(startFunc, &params, userParam);
	}

	Thread::~Thread()
	{
		Stop();
	}

	const char* Thread::GetThreadName() const
	{
		return mThreadName;
	}
	
	SysThreadId Thread::GetThreadId() const
	{
		return mThreadId;
	}

	ThreadStatus Thread::GetStatus() const
	{
		return mStatus;
	}
	
	struct ThreadDynamicData
	{
		const char* mThreadName;
		ThreadStart mpFunc;
		void* mpUserData;
	};
	
	unsigned RunFunctionInternal(void* pContext)
	{
		auto params = reinterpret_cast<ThreadDynamicData*>(pContext);
		params->mpFunc(params->mpUserData);
		return 0;
	}
	
	bool Thread::Begin(ThreadStart func, const ThreadParameters* params, void* userParam)
	{
		ThreadDynamicData* pTD = new ThreadDynamicData();
	 	pTD->mThreadName = mThreadName;
		pTD->mpFunc = func;
		pTD->mpUserData = userParam;

	 	u32 threadNum = 0;
		SysThreadId hThread = (SysThreadId)_beginthreadex(nullptr, params->mnStackSize, RunFunctionInternal, pTD, CREATE_SUSPENDED, &threadNum);
		if(!hThread)
		{
			mThreadId = nullptr;
			return false;
		}
		
		SetThreadPriority(hThread, _getpriority(params->mnPriority));
		if(params->mnProcessor == kProcessorAny)
			SetThreadAffinityMask(hThread, params->mnAffinityMask);
		else
			SetThreadIdealProcessor(hThread, params->mnProcessor);

		ResumeThread(hThread);
		mThreadId = hThread;
		mStatus = ThreadStatus::kStatusRunning;
		return true;
	}

	void Thread::Suspend()
	{
		if(mThreadId)
		{
			mStatus = ThreadStatus::kStatusSuspended;
			SuspendThread(mThreadId);
		}
	}
	void Thread::Resume()
	{
		if(mThreadId)
		{
			mStatus = ThreadStatus::kStatusRunning;
			ResumeThread(mThreadId);
		}
	}
	void Thread::Stop()
	{
		if(mThreadId)
		{
			CloseHandle(mThreadId);
			mStatus = ThreadStatus::kStatusClosed;
			mThreadId = nullptr;
		}
	}

	void Thread::sleep(int ms)
	{
		Sleep(ms);
	}
}