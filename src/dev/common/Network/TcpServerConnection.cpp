#include <Engine.Network/TcpServerConnection.h>
#include <Engine.Network/TcpServerConnectionImpl.h>

namespace rw
{
TcpServerConnection::TcpServerConnection()
	: m_impl(nullptr)
{}

TcpServerConnection::~TcpServerConnection()
{
	delete m_impl;
	m_impl = nullptr;
}

void TcpServerConnection::enqueueWrite(const void* buf_, int num)
{
	SharedBuffer* buf = new MemorySharedBuffer(buf_, num);
	m_impl->enqueueWrite(buf);
}

void TcpServerConnection::enqueueWrite(SharedBuffer* buf)
{
	m_impl->enqueueWrite(buf);
}

SharedBuffer* TcpServerConnection::dequeueRead()
{
	return m_impl->dequeueRead();
}

void TcpServerConnection::disconnect()
{
	m_impl->disconnect(true);
}

TcpServerConnectionImpl* TcpServerConnection::impl() const
{
	return m_impl;
}

void TcpServerConnection::setImpl(TcpServerConnectionImpl* impl)
{
	m_impl = impl;
}

void TcpServerConnection::onDataAvailable()
{
}

void TcpServerConnection::onConnectionEnd()
{
}

void TcpServerConnection::onDisconnect()
{
}
}