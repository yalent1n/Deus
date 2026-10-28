#include <future>
#include <Engine.Network/TcpClientConnection.h>
#include <Engine.Network/Util.h>
#include <Engine.Base/Misc/Event.h>
#include <Engine.Base/Thread/CriticalSection.h>
#include <Engine.Base/Vec.h>

#ifdef SvcCompilerMSVC
	#define WIN32_LEAN_AND_MEAN
	#include <WinSock2.h>
	#include <Windows.h>
	#include <MSWSock.h>
#endif

#include <wolfssl/ssl.h>

namespace rw
{	
	class TcpClientConnectionImpl
	{
	public:
		TcpClientConnectionImpl(TcpClientConnection* outer, SOCKET socket, bool secured, WOLFSSL_CTX* ctx, WOLFSSL* ssl)
			: m_outer(outer)
			, m_ctx(ctx)
			, m_ssl(ssl)
			, m_socket(socket)
			, m_state(ConnectionState::Active)
			, m_secured(secured)
		{}

		~TcpClientConnectionImpl()
		{
			shutdown(false);
		}

		bool isSecured() const { return m_secured; }
		bool isClosed() const { return m_state == ConnectionState::Dead || m_state == ConnectionState::Terminated; }
		
		void shutdown(bool useCallback);

		void write(const SharedBuffer* buffer);
		SharedBuffer* read(u32 size);

		void onConnectionEnd(bool useCallback);

	private:
		void closeSocket();

	private:
		TcpClientConnection* m_outer;
		WOLFSSL_CTX* m_ctx;
		WOLFSSL* m_ssl;
		SOCKET m_socket;

		CriticalSection m_writeCs;
		CriticalSection m_readCs;

		ConnectionState m_state;
		bool m_secured;
	};

	void TcpClientConnectionImpl::closeSocket()
	{
		if(m_socket != INVALID_SOCKET)
		{
			platformClose(m_socket);
			m_socket = INVALID_SOCKET;
			m_state = ConnectionState::Dead;
		}
	}

	void TcpClientConnectionImpl::shutdown(bool useCallback)
	{
		if(m_state == ConnectionState::Dead)
			return;

		onConnectionEnd(useCallback);
	}
	
	void TcpClientConnectionImpl::onConnectionEnd(bool useCallback)
	{
		if(m_state == ConnectionState::Dead)
			return;
			
		// TLS deinit
		wolfSSL_free(m_ssl);
		wolfSSL_CTX_free(m_ctx);

		// TCP deinit
		closeSocket();

		if(useCallback)
		{
			m_outer->onConnectionEnd();
		}
	}
		
	void TcpClientConnectionImpl::write(const SharedBuffer* buffer)
	{
		CriticalSectionScope _(m_writeCs);

		if(isClosed())
			return;

		WSABUF buf;
		buf.buf = (CHAR*) buffer->bytes();
		buf.len = (ULONG) buffer->size();

		if(!m_secured)
		{
			DWORD dwBytesSent = 0;
			if(WSASend(m_socket, &buf, 1, &dwBytesSent, 0, NULL, NULL) < 0)
			{
				int err = WSAGetLastError();
				switch(err)
				{
					case ERROR_IO_PENDING: // should not receive that
						break;
					default:
						m_state = ConnectionState::Terminated;
						shutdown(true);
						return;
				}
			}
		}
		else
		{
			int ret = wolfSSL_write(m_ssl, buf.buf, (int) buf.len);
			if(ret <= 0)
			{
				m_state = ConnectionState::Disconnected;
				shutdown(true);
			}
		}
	}

	SharedBuffer* TcpClientConnectionImpl::read(u32 size)
	{
		CriticalSectionScope _(m_readCs);

		if(!size)
			return nullptr;

		WSABUF wsaBuf;
		wsaBuf.buf = (CHAR*) HeapAllocGlobal(size);
		wsaBuf.len = size;

		DWORD dwBytesRead = 0;
		if(!m_secured)
		{
			DWORD dwFlags = 0;
			if(WSARecv(m_socket, &wsaBuf, 1, &dwBytesRead, &dwFlags, NULL, NULL) < 0)
			{
				int err = WSAGetLastError();
				EERRORF("(socket) receive failed on socket %u (%x)", m_socket, err);
				switch(err)
				{
					case ERROR_IO_PENDING: // should not receive that
						break;
					case WSAEWOULDBLOCK:
						// No data available
						HeapFreeGlobal(wsaBuf.buf);
						return nullptr;
					case WSAECONNRESET:
					case WSAECONNABORTED:
					case WSAENETRESET:
						m_state = ConnectionState::Terminated;
						HeapFreeGlobal(wsaBuf.buf);
						return nullptr;
					default:
						m_state = ConnectionState::Terminated;
						HeapFreeGlobal(wsaBuf.buf);
						shutdown(true);
						return nullptr;
				}
			}
		}
		else
		{
			int ret = wolfSSL_read(m_ssl, wsaBuf.buf, (int)wsaBuf.len);
			if(ret <= 0)
			{
				shutdown(true);
				return nullptr;
			}

			dwBytesRead = ret;
		}
		
		return new MemorySharedBuffer(wsaBuf.buf, dwBytesRead);
	}

	TcpClientConnection::TcpClientConnection()
		: m_impl(nullptr)
	{}

	TcpClientConnection::~TcpClientConnection()
	{
		if(m_impl)
		{
			m_impl->shutdown(false);
			delete m_impl;
			m_impl = nullptr;
		}
	}

	bool TcpClientConnection::isDead() const
	{
		return m_impl->isClosed();
	}
	
	void TcpClientConnection::disconnect()
	{
		if(m_impl)
		{
			m_impl->shutdown(true);
			delete m_impl;
			m_impl = nullptr;
		}
	}

	void TcpClientConnection::writeFully(const void* buf, u32 size)
	{
		SharedBuffer* buffer = new MemorySharedBuffer(buf, size);
		m_impl->write(buffer);
	}

	void TcpClientConnection::writeFully(SharedBuffer* buffer)
	{
		m_impl->write(buffer);
	}

	SharedBuffer* TcpClientConnection::read(u32 size)
	{
		return m_impl->read(size);
	}
		
	TcpClientConnection* tcpConnectHost(const SocketAddress& address, bool enableTLS, TcpClientConnection::CreateCallback callback)
	{
		int ret;

		SOCKET socket = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
		if(socket == INVALID_SOCKET)
			return nullptr;

		ret = SetHandleInformation((HANDLE)socket, HANDLE_FLAG_INHERIT, 0);
		EASSERTF(ret, "(socket) SetHandleInformation() failed on socket %u", socket);

		ret = ::connect(socket, address.as<const sockaddr>(), address.length);
		DWORD __error__ = WSAGetLastError();
		if(ret == SOCKET_ERROR)
		{
			platformClose(socket);
			EERRORF("(socket) connect() failed on socket %u", socket);
			return nullptr;
		}
		
		WOLFSSL_CTX* ctx = wolfSSL_CTX_new(wolfTLSv1_3_client_method());
		if (!ctx)
		{
			EERROR("(socket) failed to setup TLS context");
			return nullptr;
		}

		wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);

		WOLFSSL* ssl = wolfSSL_new(ctx);
		if(!ssl)
		{
			EERROR("(socket) failed to setup TLS instance");
			return nullptr;
		}

		wolfSSL_set_fd(ssl, (int) socket);

		ret = wolfSSL_connect(ssl);
		if(ret != SSL_SUCCESS)
		{
			int err = wolfSSL_get_error(ssl, ret);
			EERRORF("(socket) failed to connect using TLS (err=%d)", err);
			return nullptr;
		}

		TcpClientConnection* conn = callback();
		EASSERT(conn, "create callback returned null pointer");

		TcpClientConnectionImpl* impl = new TcpClientConnectionImpl(conn, socket, enableTLS, ctx, ssl);
		conn->m_impl = impl;
		conn->onConnectionBegin();
		return conn;
	}
}
