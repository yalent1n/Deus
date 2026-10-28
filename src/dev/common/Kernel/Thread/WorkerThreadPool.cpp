#include <Engine.Base/Thread/WorkerThreadPool.h>
#include <Engine.Base/Thread/CriticalSection.h>
#include <Engine.Base/String/String.h>

#ifdef SvcCompilerMSVC
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif

void workerThreadRun(void* param)
{
	using namespace rw;
	
	WorkerThreadPool* threadPool = (WorkerThreadPool*) param;

	DWORD bytesTransferred = 0;
	ULONG_PTR completionKey = 0;
	LPOVERLAPPED overlapped = 0;

	bool loop = true;
	while(loop)
	{
		BOOL success = GetQueuedCompletionStatus(threadPool->completionPortHandle(),
												 &bytesTransferred,
												 &completionKey, 
												 &overlapped,
												 INFINITE);

		AsyncIoRequest* request = (AsyncIoRequest*) overlapped;
		if(success)
		{
			if(overlapped)
			{
				request->OnCompletion(u32(bytesTransferred), completionKey);
			}
			else
			{
				loop = false;
				break;
			}
		}
		else
		{
			DWORD code = GetLastError();
			switch(code)
			{
			case ERROR_ABANDONED_WAIT_0:
				loop = false; // Completion port closed
				break;

			case ERROR_MORE_DATA:
				break;

			default:
			case ERROR_BROKEN_PIPE:
			case ERROR_OPERATION_ABORTED:
				if(request)
					request->OnError(u32(code));
				break;
			}
		}
	}
}

namespace rw
{	
WorkerThreadPool::WorkerThreadPool(const char* name, u32 threadCount, u32 stackSize)
	: m_name(name)
	, m_iocp(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0))
	, m_threadCount(threadCount)
{
	spawnThreads(stackSize);
}

WorkerThreadPool::~WorkerThreadPool()
{
	CriticalSectionScope _(m_cs);
	for(u32 i = 0; i < m_threadCount; ++i)
	{
		postCompletion(0, 0, NULL);
	}

	for(Thread* t : m_threads)
		delete t;
	
	CloseHandle(m_iocp);
}

void WorkerThreadPool::postCompletion(u32 bytesTransferred, u64 completionKey, LPOVERLAPPED Ol)
{
	PostQueuedCompletionStatus(m_iocp, DWORD(bytesTransferred), ULONG_PTR(completionKey), Ol);
}

bool WorkerThreadPool::createPort(void* handle)
{
	return !!CreateIoCompletionPort(HANDLE(handle), HANDLE(m_iocp), 0, 0);
}

void WorkerThreadPool::spawnThreads(unsigned stackSize)
{
	CriticalSectionScope _(m_cs);
	
	for(unsigned i = 0; i < m_threadCount; i++)
	{
		StringBuilder<64> name;
		name << m_name << i;
		Thread* t = new Thread(
			name.detach(),
			stackSize,
			workerThreadRun, 
			this);

		m_threads.push_back(t);
	}
}
}