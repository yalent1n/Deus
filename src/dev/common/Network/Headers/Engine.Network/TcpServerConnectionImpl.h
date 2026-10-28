#pragma once
#include "TcpServerConnection.h"
#include <Engine.Base/Vec.h>
#include <Engine.Base/Thread/WorkerThreadPool.h>
#include <Engine.Base/Thread/CriticalSection.h>

#define WOLFSSL_NO_OPTIONS_H 1
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/ssl.h>

namespace rw
{
class TcpServerConnectionImpl
{
	class TcpServerConnectionReader;
	class TcpServerConnectionWriter;
	friend class TcpServerConnectionReader;	
	friend class TcpServerConnectionWriter;

public:
	TcpServerConnectionImpl(TcpServerConnection* outer, SOCKET socket, WorkerThreadPool* threadPool, u32 readBlockSize, bool secured, WOLFSSL* tls);
	virtual ~TcpServerConnectionImpl();

	SOCKET getSocket() const;
	bool isDead() const;

	void begin(SharedBuffer* buffer, u32 size);
	void shutdown(bool useCallback);
	void closeSocket();
	void disconnect(bool useCallback);

public:
	SharedBuffer* dequeueRead();
	void enqueueWrite(SharedBuffer* buf);

private:
	void onReadCompletion(SharedBuffer* buf, u32 size);
	void onWriteCompletion(SharedBuffer* buf);

private:
	struct CompletedRead
	{
		SharedBuffer* buf;
		u32 size;
	};
	struct PendingWrite
	{
		SharedBuffer* buf;
	};
private:
	TcpServerConnection* m_outer;
	WOLFSSL* m_tls;
	WorkerThreadPool* m_threadPool;
	SOCKET m_socket;
	u32 m_id;
	u32 m_readBlockSize;

	CriticalSection m_readsCs;
	Vec<CompletedRead> m_completedReads;
	ScopedRef<TcpServerConnectionReader> m_reader;

	CriticalSection m_writesCs;
	Vec<PendingWrite> m_pendingWrites;
	ScopedRef<TcpServerConnectionWriter> m_writer;

	ConnectionState m_state;
	bool m_secured;
};
}