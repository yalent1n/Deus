#pragma once
#include "Bundle.h"
#include <Engine.Base/Thread/WorkerThreadPool.h>
#include <Engine.Base/Buffer/SharedBuffer.h>
#include "TcpServerConnection.h"

namespace rw
{
class TcpServerImpl;

// Async TCP server implementation.
class TcpServer
{
	friend class TcpServerImpl;
	
public:
	FB_NETWORK_API TcpServer(u16 portNumber, WorkerThreadPool* threadPool, bool secured, u32 readBlockSize = ConnectionReadBlockSize_Quick);
	FB_NETWORK_API virtual ~TcpServer();

	FB_NETWORK_API u32 getReadBlockSize() const;
	FB_NETWORK_API void setReadBlockSize(u32 readSize);
	
	// Closes the server and reaps connections.
	FB_NETWORK_API void shutdown();

	// Accepts connections.
	FB_NETWORK_API void update();

	// Iterates through each connection.
	FB_NETWORK_API void forEachConnection(void(*fn)(TcpServerConnection*));

	// IMPLEMENTATION OVERRIDE
	// Creates a new connection, based on the service.
	virtual TcpServerConnection* newConnection() = 0;

	// IMPLEMENTATION OVERRIDE
	// When a new connection is made.
	virtual void onConnect(TcpServerConnection* connection) = 0;

	// IMPLEMENTATION OVERRIDE
	// When a connection has dropped.
	virtual void onDisconnect(TcpServerConnection* connection) = 0;

private:
	ScopedRef<TcpServerImpl> m_impl;
	u32 m_readBlockSize;
	u16 m_portNumber;
};
}