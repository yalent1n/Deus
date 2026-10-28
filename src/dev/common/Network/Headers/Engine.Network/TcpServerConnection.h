#pragma once
#include "Bundle.h"
#include "SocketBase.h"
#include <Engine.Base/Buffer/SharedBuffer.h>

namespace rw
{
class TcpServerConnectionImpl;

// Base class for a un/secured TCP connections on the server side.
class TcpServerConnection
{
	friend class TcpServerConnectionImpl;

public:
	FB_NETWORK_API TcpServerConnection();
	FB_NETWORK_API virtual ~TcpServerConnection();
	
	FB_NETWORK_API void enqueueWrite(const void* buf, int num);
	FB_NETWORK_API void enqueueWrite(SharedBuffer* buf);

	FB_NETWORK_API SharedBuffer* dequeueRead();

	FB_NETWORK_API void disconnect();

	FB_NETWORK_API TcpServerConnectionImpl* impl() const;
	FB_NETWORK_API void setImpl(TcpServerConnectionImpl* impl);

	FB_NETWORK_API virtual void onDataAvailable();
	FB_NETWORK_API virtual void onConnectionEnd();
	FB_NETWORK_API virtual void onDisconnect();

private:
	TcpServerConnectionImpl* m_impl;
};
}