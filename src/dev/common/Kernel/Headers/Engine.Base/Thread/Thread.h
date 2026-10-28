#pragma once
#include "../Bundle.h"

namespace rw
{
    // The underlying OS thread handle type.
	typedef void* SysThreadId;
    typedef u64	ThreadAffinityMask;    
    typedef void(*ThreadStart)(void*);

    inline constexpr int kProcessorDefault = 64;
	inline constexpr int kProcessorAny = -2;
	inline constexpr ThreadAffinityMask kThreadAffinityMaskAll = ~0U;

	enum class Priority : u8
	{
		BelowNormal,
		Normal,
		AboveNormal,
		High,
		TimeCritical,
		Default = Normal
	};
	
	struct ThreadParameters
	{
		u32 mnStackSize;
		Priority mnPriority;
		int mnProcessor;
		ThreadAffinityMask mnAffinityMask;
	};

	enum class ThreadStatus : u8
	{
		kStatusInactive,
		kStatusSuspended,
		kStatusRunning,
		kStatusClosed
	};
    
    class Thread
    {
    public:
        FB_BASE_API Thread(const char* name,
            u32 stackSize,
            ThreadStart startFunc,
            void* userParam = nullptr,
            int processor = kProcessorDefault,
            Priority priority = Priority::Default,
            ThreadAffinityMask affinity = kThreadAffinityMaskAll
        );

        FB_BASE_API ~Thread();

        FB_BASE_API const char*         GetThreadName() const;
        FB_BASE_API SysThreadId         GetThreadId() const;
        FB_BASE_API ThreadStatus        GetStatus() const;

        // Begins a thread and returns true if it's succeded, otherwise false.
		// @param func: Function that will be thread executed.
		FB_BASE_API bool Begin(ThreadStart func, const ThreadParameters* params, void* userParam = nullptr);
		FB_BASE_API void Suspend();
		FB_BASE_API void Resume();
		FB_BASE_API void Stop();

        // Sleeps for given milliseconds.
        FB_BASE_API static void sleep(int ms);

    private:
        const char* mThreadName;
		SysThreadId mThreadId;
		ThreadStatus mStatus;
    };
}