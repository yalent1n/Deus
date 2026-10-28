#pragma once
#include "Thread.h"
#include "CriticalSection.h"
#include "../Misc/FastDelegate.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <vector>

namespace rw
{
struct AsyncIoRequest
{
	typedef FastDelegate<void(u32, u64)> CompletedFunc_t;
	typedef FastDelegate<void(u32)> ErrorFunc_t;

	AsyncIoRequest() {
		ZeroMemory(this, sizeof(AsyncIoRequest));
	}

	OVERLAPPED Ol;
	CompletedFunc_t OnCompletion;
	ErrorFunc_t OnError;
};

class WorkerThreadPool
{
public:
	FB_BASE_API WorkerThreadPool(const char* name = "work", u32 threadCount = 8, u32 stackSize = 1048576u);
	FB_BASE_API ~WorkerThreadPool();

	void* completionPortHandle() const { return m_iocp; }

	FB_BASE_API void postCompletion(u32 bytesTransferred, u64 completionKey, LPOVERLAPPED Ol);
	FB_BASE_API bool createPort(void* handle);

private:
	void spawnThreads(unsigned stackSize);

private:
	const char* m_name;
	void* m_iocp;
	CriticalSection m_cs;
	std::vector<Thread*> m_threads;
	u32 m_threadCount;
};
}
