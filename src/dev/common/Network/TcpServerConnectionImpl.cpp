#include <Engine.Network/TcpServerConnectionImpl.h>
#include <Engine.Network/SocketBase.h>

namespace rw
{
static inline u32 m_connectionCounter = 0;

class TcpServerConnectionImpl::TcpServerConnectionReader : public AsyncIoRequest
{
public:
	TcpServerConnectionReader(SOCKET socket, TcpServerConnectionImpl* outer)
		: m_connection(outer)
		, m_socket(socket)
		, m_currentBuffer(nullptr)
		, m_currentState(IoState::Open)
	{
		OnCompletion = makeDelegate(this, &TcpServerConnectionReader::Completion);
		OnError = makeDelegate(this, &TcpServerConnectionReader::Error);
	}

	bool isClosed() const { return m_currentState == IoState::Terminated || m_currentState == IoState::Closed; }
	bool isPending() const { return m_currentState == IoState::Flight; }

	void cancel()
	{
		if(m_currentState == IoState::Flight)
		{
			m_currentState = IoState::Cancelled;
			CancelIoEx((HANDLE)m_socket, &Ol);
			return;
		}

		m_currentState = IoState::Terminated;		
	}

	void newRead()
	{
		m_currentBuffer = new MemorySharedBuffer(m_connection->m_readBlockSize);

		if(m_connection->m_secured)
		{
			int ret;
			for(;;)
			{
				ret = wolfSSL_read(m_connection->m_tls, m_currentBuffer->bytes(), m_currentBuffer->size());
				if(ret > 0)
					break;
				
				if(ret == 0)
				{
					m_connection->m_threadPool->postCompletion(0, 0, &Ol);
					return;
				}

				int err = wolfSSL_get_error(m_connection->m_tls, ret);
				if(err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE)
				{
					m_currentState = IoState::Failed;
					m_connection->shutdown(true);
					return;
				}
			}

			m_connection->m_threadPool->postCompletion(ret, 0, &Ol);
		}
		else
		{
			WSABUF buf;
			buf.buf = (CHAR*)m_currentBuffer->bytes();
			buf.len = (ULONG)m_currentBuffer->size();

			m_currentState = IoState::Flight;

			DWORD dwBytesReceived = 0;
			DWORD dwFlags = 0;

			if(WSARecv(m_socket, &buf, 1, &dwBytesReceived, &dwFlags, &Ol, NULL) == SOCKET_ERROR)
			{
				switch(WSAGetLastError())
				{
				case ERROR_IO_PENDING:
					return;
				case WSAECONNRESET:
				case WSAECONNABORTED:
					m_currentState = IoState::Terminated;
					m_connection->shutdown(true);
					return;
				default:
					m_currentState = IoState::Failed;
					m_connection->shutdown(true);
					return;
				}
			}
		}
	}

	void Completion(u32 bytesTransferred, u64 completionKey)
	{
		if(m_currentState == IoState::Cancelled)
		{
			m_currentState = IoState::Closed;
			return;
		}

		if(m_currentState == IoState::Closed)
			return;

		if(bytesTransferred == 0)
		{
			m_currentState = IoState::Closed;
			m_connection->shutdown(true);
			return;
		}

		m_currentState = IoState::Completed;
		m_connection->onReadCompletion(m_currentBuffer.get(), bytesTransferred);

		switch(m_currentState)
		{
		case IoState::Open:
		case IoState::Completed:
			newRead();
			return;
		default:
			return;
		}
	}

	void Error(u32 error)
	{	
		if(m_currentState == IoState::Cancelled)
		{
			m_currentState = IoState::Closed;		
		}
		else if(m_currentState == IoState::Flight)
		{
			EASSERT(error == ERROR_NETNAME_DELETED, "Unexpected error in read");
		}
		else
		{
			if(error == ERROR_NETNAME_DELETED || error == WSAECONNRESET)
				m_currentState = IoState::Terminated;
			else
				m_currentState = IoState::Closed;
		}

		m_connection->shutdown(true);
	}

private:
	TcpServerConnectionImpl* m_connection;
	SOCKET m_socket;
	ScopedRef<SharedBuffer> m_currentBuffer;
	CriticalSection m_cs;
	IoState m_currentState;
};

class TcpServerConnectionImpl::TcpServerConnectionWriter : public AsyncIoRequest
{
public:
	TcpServerConnectionWriter(SOCKET socket, TcpServerConnectionImpl* connection)
		: m_connection(connection)
		, m_socket(socket)
		, m_currentBuffer(nullptr)
		, m_currentState(IoState::Open)
	{
		OnCompletion = makeDelegate(this, &TcpServerConnectionWriter::Completion);
		OnError = makeDelegate(this, &TcpServerConnectionWriter::Error);
	}

	bool isClosed() const { return m_currentState == IoState::Closed || m_currentState == IoState::Terminated; }
	bool isPending() const { return m_currentState == IoState::Flight; }

	void cancel()
	{
		if(m_currentState == IoState::Flight)
		{
			m_currentState = IoState::Cancelled;
			CancelIoEx((HANDLE)m_socket, &Ol);
			return;
		}

		m_currentState = IoState::Terminated;
	}

	// Returns true if attempt was successful, otherwise false.
	bool issue(const PendingWrite& write)
	{
		CriticalSectionScope _(m_cs);

		if(m_currentState == IoState::Flight)
			return false;

		if(isClosed())
			return false;

		m_currentState = IoState::Flight;
		m_currentBuffer = write.buf;

		if(m_connection->m_secured)
		{
			int ret;
			for(;;)
			{
				ret = wolfSSL_write(m_connection->m_tls, m_currentBuffer->bytes(), m_currentBuffer->size());
				if(ret > 0)
					break;
				
				int err = wolfSSL_get_error(m_connection->m_tls, ret);
				if(err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE)
				{
					m_currentState = IoState::Failed;
					m_connection->shutdown(true);
					return false;
				}
			}

			m_connection->m_threadPool->postCompletion(ret, 0, &Ol);
		}
		else
		{
			WSABUF buf;
			buf.buf = (CHAR*)m_currentBuffer->bytes();
			buf.len = (ULONG)m_currentBuffer->size();

			DWORD dwBytesSent = 0;

			if(WSASend(m_socket, &buf, 1, &dwBytesSent, 0, &Ol, NULL) == SOCKET_ERROR)
			{
				switch(WSAGetLastError())
				{
				case ERROR_IO_PENDING:
					break;
				case WSAECONNRESET:
				case WSAECONNABORTED:
				case WSAENETRESET:
					m_currentState = IoState::Terminated;
					m_connection->shutdown(true);
					return false;
				default:
					m_currentState = IoState::Failed;
					m_connection->shutdown(true);
					return false;
				}
			}
		}

		return true;
	}

	void Completion(u32 bytesTransferred, u64 completionKey)
	{
		if(m_currentState != IoState::Flight)
		{
			// no pending write has attempted
			return;
		}

		m_currentState = IoState::Completed;
		m_connection->onWriteCompletion(m_currentBuffer.get());
	}

	void Error(u32 error)
	{
		ELOGF("write handle error: %u", error);
	}

private:
	TcpServerConnectionImpl* m_connection;
	SOCKET m_socket;
	ScopedRef<SharedBuffer> m_currentBuffer;
	CriticalSection m_cs;
	IoState m_currentState;
};

TcpServerConnectionImpl::TcpServerConnectionImpl(TcpServerConnection* outer, SOCKET socket, WorkerThreadPool* threadPool, u32 readBlockSize, bool secured, WOLFSSL* tls)
	: m_outer(outer)
	, m_tls(tls)
	, m_threadPool(threadPool)
	, m_socket(socket)
	, m_readBlockSize(readBlockSize)
	, m_reader(new TcpServerConnectionReader(socket, this))
	, m_writer(new TcpServerConnectionWriter(socket, this))
	, m_state(ConnectionState::Active)
	, m_secured(secured)
{
	m_id = ++m_connectionCounter;
}

TcpServerConnectionImpl::~TcpServerConnectionImpl()
{
	shutdown(false);
}

SOCKET TcpServerConnectionImpl::getSocket() const
{
	return m_socket;
}
	
bool TcpServerConnectionImpl::isDead() const
{
	return m_state == ConnectionState::Dead || m_state == ConnectionState::Terminated;
}

void TcpServerConnectionImpl::begin(SharedBuffer* buffer, u32 size)
{
	if(buffer)
	{
		//FB_DEBUG("Begin connection #%u - bytes", m_id);
		onReadCompletion(buffer, size);
	}
	else
	{
		if(m_secured)
		{
			int ret;
			for(;;)
			{
				//ret = wolfSSL_accept_TLSv13(m_tls);
				ret = wolfSSL_accept(m_tls);
				if(ret == WOLFSSL_SUCCESS)
					break;
			}
		}
		//FB_DEBUG("Begin connection #%u - no bytes", m_id);
	}

	if(m_state == ConnectionState::Active)
	{
		m_reader->newRead();
	}
}

void TcpServerConnectionImpl::shutdown(bool useCallback)
{
	if(m_state == ConnectionState::Dead)
		return;

	bool done = m_reader->isClosed() && !m_writer->isPending();
	if(done)
	{
		closeSocket();
		m_outer->onConnectionEnd();
		m_state = ConnectionState::Dead;
	}
}

void TcpServerConnectionImpl::closeSocket()
{
	if(m_state == ConnectionState::Dead)
		return;

	wolfSSL_free(m_tls);
	m_tls = nullptr;

	platformClose(m_socket);
	m_socket = INVALID_SOCKET;

	--m_connectionCounter;
;	m_state = ConnectionState::Dead;
}

void TcpServerConnectionImpl::disconnect(bool useCallback)
{
	m_state = ConnectionState::Dead;
	
	if(!m_writer.isNull())
		m_writer->cancel();

	if(!m_reader.isNull())
		m_reader->cancel();
	
	shutdown(useCallback);
}

SharedBuffer* TcpServerConnectionImpl::dequeueRead()
{
	CriticalSectionScope _(m_readsCs);

	if(m_completedReads.empty())
		return nullptr;	

	CompletedRead read = m_completedReads.front();
	m_completedReads.pop_front();
	return new MemorySharedBuffer(read.buf->bytes(), read.size);
}

void TcpServerConnectionImpl::enqueueWrite(SharedBuffer* buf)
{
	PendingWrite write;
	write.buf = buf;

	CriticalSectionScope _(m_writesCs);

	if(m_pendingWrites.empty())
	{
		if(m_writer->issue(write))
		{
			buf->releaseResources();
			return;
		}
	}

	m_pendingWrites.push_back(write);

	const PendingWrite& queuedWrite = m_pendingWrites.front();
	if(m_writer->issue(queuedWrite))
	{
		queuedWrite.buf->releaseResources();
		m_pendingWrites.pop_front();
	}
}

void TcpServerConnectionImpl::onReadCompletion(SharedBuffer* buf, u32 size)
{
	CompletedRead read;
	read.buf = buf;
	read.size = size;

	CriticalSectionScope _(m_readsCs);
	m_completedReads.push_back(read);
	m_outer->onDataAvailable();
}

void TcpServerConnectionImpl::onWriteCompletion(SharedBuffer* buf)
{

}
}