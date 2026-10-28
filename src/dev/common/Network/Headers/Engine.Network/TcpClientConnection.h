#pragma once
#include "Bundle.h"
#include "SocketBase.h"
#include <Engine.Base/Thread/WorkerThreadPool.h>
#include <Engine.Base/Buffer/SharedBuffer.h>
#include <Engine.Base/Misc/FastDelegate.h>

namespace rw
{
class TcpClientConnectionImpl;

// Unsecured TCP connection on the client side.
class TcpClientConnection
{
	friend class TcpClientConnectionImpl;
	
public:
	typedef FastDelegate<TcpClientConnection*()> CreateCallback;
	friend TcpClientConnection* tcpConnectHost(const SocketAddress&, bool, TcpClientConnection::CreateCallback);

	// The client connection should use less memory for reading
	// than the server (64kiB).
	// In imagination, the client connection won't get large data
	// from the server, only if it receives like a big file,
	// but that depends on the service.
	enum : u32 { ReadBlockSize = 4096 };
	
	FB_NETWORK_API TcpClientConnection();
	FB_NETWORK_API virtual ~TcpClientConnection();

	FB_NETWORK_API bool isDead() const;
	FB_NETWORK_API void disconnect();

	FB_NETWORK_API void writeFully(const void* buf, u32 size);
	FB_NETWORK_API void writeFully(SharedBuffer* buffer);
	FB_NETWORK_API SharedBuffer* read(u32 size = ReadBlockSize);

	virtual void onConnectionBegin() = 0;
	virtual void onConnectionEnd() = 0;
	
private:
	TcpClientConnectionImpl* m_impl = nullptr;
};

FB_NETWORK_API TcpClientConnection* tcpConnectHost(const SocketAddress& address, bool enableTLS, TcpClientConnection::CreateCallback callback);

class ScopedConnection
{
public:
	ScopedConnection(const SocketAddress& address, bool enableTLS, TcpClientConnection::CreateCallback callback)
		: m_connection(tcpConnectHost(address, enableTLS, callback))
	{}

	~ScopedConnection()
	{
		if(m_connection)
		{
			m_connection->disconnect();
			delete m_connection;
			m_connection = nullptr;
		}
	}

	bool isConnected()
	{
		return !!(m_connection);
	}

	TcpClientConnection* operator->()
	{
		return m_connection;
	}
	
	const TcpClientConnection* operator->() const
	{
		return m_connection;
	}

private:
	TcpClientConnection* m_connection;
};
}
