#include <Engine.Network/Bundle.h>
#include <Engine.Network/TcpServer.h>
#include <Engine.Network/TcpServerConnectionImpl.h>
#include <Engine.Network/Util.h>
#include <Engine.Base/Misc/Event.h>

#ifdef SvcCompilerMSVC
	// #include <winsock.h>
	#include <MSWSock.h>
	#pragma comment(lib, "mswsock.lib")
#endif

namespace rw
{
class TcpAccept : public AsyncIoRequest
{
	//static constexpr u32 AddressLength = sizeof(SOCKADDR_STORAGE) + 16;
	static constexpr u32 AddressLength = sizeof(sockaddr_in) + 16;
	
public:
	TcpAccept(WorkerThreadPool* threadPool, u16 portNumber, TcpServerImpl* server, u32* readBlockSizePtr, bool secured)
		: m_server(server)
		, m_threadPool(threadPool)
		, m_readBlockSizePtr(readBlockSizePtr)
		, m_tlsCtx(nullptr)
		, m_closeEvent(true, false, "CloseEvent")
		, m_isClosing(false)
		, m_secured(secured)
	{
		OnCompletion = makeDelegate(this, &TcpAccept::Completion);
		OnError = makeDelegate(this, &TcpAccept::Error);

		// here we just create the accept buffer that's large enough to store the
		// local and remote address (and read block size won't get changed to some obritary value).
		(void) swapAcceptBuffers();
		createListenSocket(portNumber);
		EASSERT(threadPool->createPort((HANDLE)m_listenSocket), "Failed to associate listen socket with the thread pool");

		if(secured)
		{
			m_tlsCtx = wolfSSL_CTX_new(wolfTLSv1_3_server_method());
			EASSERT(m_tlsCtx, "Failed to create TLS context");
			//wolfSSL_CTX_set_verify(m_tlsCtx, SSL_VERIFY_NONE, NULL);

			auto certBuf = Net_GetCertPem();
			EASSERT(wolfSSL_CTX_use_certificate_buffer(m_tlsCtx, (const u8*) certBuf->bytes(), (long) certBuf->size(), SSL_FILETYPE_PEM) == WOLFSSL_SUCCESS, 
				"Failed to use certificate in a TLS context");
			
			auto keyBuf = Net_GetKeyPem();
			EASSERT(wolfSSL_CTX_use_PrivateKey_buffer(m_tlsCtx, (const u8*) keyBuf->bytes(), (long) keyBuf->size(), SSL_FILETYPE_PEM) == WOLFSSL_SUCCESS, 
				"Failed to use private key in a TLS context");

			ELOG("(secured tcp) Managed to set up TLS context");
		}
		
		GUID GuidAcceptEx = WSAID_ACCEPTEX;
		DWORD Bytes;
		WSAIoctl(m_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &m_AcceptFunc, sizeof(m_AcceptFunc), &Bytes, NULL, NULL);
		EASSERT(m_AcceptFunc, "Failed to get extension pointer AcceptEx");

		createAcceptSocket();
	}

	~TcpAccept()
	{
		closeSockets();
		m_closeEvent.testState(5000); // wait

		if(m_tlsCtx)
		{			
			wolfSSL_CTX_free(m_tlsCtx);
			m_tlsCtx = nullptr;
		}
	}

	void createListenSocket(u16 portNumber);
	void createAcceptSocket();

	void closeListenSocket();
	void closeAcceptSocket();
	void closeSockets();

	SharedBuffer* swapAcceptBuffers();
	
	void*	buffer() const;
	u32		bufferSize() const;

	void Completion(u32 bytesTransferred, u64 completionKey);
	void Error(u32 code);

private:
	LPFN_ACCEPTEX m_AcceptFunc;

	TcpServerImpl* m_server;
	WorkerThreadPool* m_threadPool;
	u32* m_readBlockSizePtr;
	SharedBuffer* m_buffer;
	WOLFSSL_CTX* m_tlsCtx;
	SOCKET m_acceptSocket;
	SOCKET m_listenSocket;
	Event m_closeEvent;
	bool m_isClosing;
	bool m_secured;
};

class TcpServerImpl
{
	friend class TcpAccept;
	friend class TcpServer;

public:
	TcpServerImpl(TcpServer* outer, u16 portNumber, WorkerThreadPool* threadPool, bool secured)
		: m_outer(outer)
		, m_accept(threadPool, portNumber, this, &outer->m_readBlockSize, secured)
	{}

	void shutdown();
	void update();
	bool updateConnections();

	void onConnect(TcpServerConnection* connection);

private:
	TcpServer* m_outer;
	TcpAccept m_accept;
	CriticalSection m_connCs;
	Vec<TcpServerConnection*> m_connections;
};

void* TcpAccept::buffer() const
{
	return m_buffer->bytes();
}

u32 TcpAccept::bufferSize() const
{
	return m_buffer->size();
}

void TcpAccept::createListenSocket(u16 portNumber)
{
	int ret;
	SOCKET listenSocket = winsock::pullup(SOCK_STREAM, IPPROTO_TCP, true, true);

	int bits = 1;
	ret = setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&bits, sizeof(bits));
	EASSERT(!(ret < 0), "setsockopt(SO_REUSEADDR) failed");

	SocketAddress addr(0u, portNumber);
	ret = bind(listenSocket, addr.as<const sockaddr>(), addr.length);
	EASSERT(ret != SOCKET_ERROR, "bind failed");

	ret = listen(listenSocket, SOMAXCONN);
	EASSERT(ret != SOCKET_ERROR, "listen failed");

	ELOGF("Listening on port %u", portNumber);
	m_listenSocket = listenSocket;
}

void TcpAccept::createAcceptSocket()
{
	DWORD dwBytes = 0;
	m_acceptSocket = winsock::pullup(SOCK_STREAM, IPPROTO_TCP, true, true);

	EASSERTF(m_threadPool->createPort((void*)m_acceptSocket), "(socket) failed to create port on socket %u", m_acceptSocket);

	void* buf = buffer();
	u32 bufSize = 0;
	if(!m_secured) {
		bufSize = bufferSize() - (2 * AddressLength);
	}

	if(!m_AcceptFunc(m_listenSocket,
	   m_acceptSocket,
	   buf,
	   bufSize,
	   AddressLength,
	   AddressLength,
	   &dwBytes,
	   &Ol))
	{
		DWORD wsaErr = WSAGetLastError();
		if(wsaErr != ERROR_IO_PENDING)
		{
			EASSERTF(false, "AcceptEx failed (%x)", wsaErr);
		}
	}
}

void TcpAccept::closeListenSocket()
{
	if(m_listenSocket != INVALID_SOCKET)
	{
		platformClose(m_listenSocket);
		m_listenSocket = INVALID_SOCKET;
	}
}

void TcpAccept::closeAcceptSocket()
{
	if(m_acceptSocket != INVALID_SOCKET)
	{
		platformClose(m_acceptSocket);
		m_acceptSocket = INVALID_SOCKET;
	}
}

void TcpAccept::closeSockets()
{
	m_isClosing = true;
	
	closeAcceptSocket();
	closeListenSocket();
}

SharedBuffer* TcpAccept::swapAcceptBuffers()
{
	SharedBuffer* buf = m_buffer;
	m_buffer = new MemorySharedBuffer(*m_readBlockSizePtr + (2 * AddressLength));
	return buf;
}

void TcpAccept::Completion(u32 bytesTransferred, u64 completionKey)
{
	int ret;

	// have to do this, otherwise the accepted socket won't be in a connected state.
	ret = setsockopt(m_acceptSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (const char*)&m_listenSocket, sizeof(m_listenSocket));
	if(ret == SOCKET_ERROR)
	{
		EASSERT(false, "setsockopt(SO_UPDATE_ACCEPT_CONTEXT) failed");
	}

	WOLFSSL* tls = nullptr;

	if(m_secured)
	{
		tls = wolfSSL_new(m_tlsCtx);

		if(!tls) {
			EERROR("Failed to create TLS connection");
			return;
		}
	
		if(wolfSSL_set_fd(tls, (int)m_acceptSocket) != WOLFSSL_SUCCESS) {
			EERROR("Failed to set FD on TLS connection");
			return;
		}
	}

	TcpServerConnection* connection = m_server->m_outer->newConnection();

	SharedBuffer* buf = nullptr;

	// donate the current buffer to the newly accepted connection,
	// and create a new buffer for the next incoming connection.
	if(bytesTransferred)
		buf = swapAcceptBuffers();
	
	TcpServerConnectionImpl* impl = new TcpServerConnectionImpl(connection, m_acceptSocket, m_threadPool, m_server->m_outer->getReadBlockSize(), m_secured, tls);
	connection->setImpl(impl);

	m_server->onConnect(connection);
	impl->begin(buf, bytesTransferred);

	createAcceptSocket();
}

void TcpAccept::Error(u32 code)
{
	if(m_isClosing)
	{
		m_closeEvent.set();
		return;
	}

	EERRORF("Accept error %u", code);

	closeAcceptSocket();
	createAcceptSocket();
}

void TcpServerImpl::shutdown()
{
	{
		CriticalSectionScope _(m_connCs);

		for(TcpServerConnection* connection : m_connections)	
			connection->disconnect();
	}

	// int retry = 0;
	// do
	// {
	// 	Sleep(100);
	// 	if(!updateConnections())
	// 	{
	// 		m_connections.clear();
	// 		break;
	// 	}
	// } while(++retry < 5);

	m_accept.closeSockets();
}

void TcpServerImpl::update()
{
	updateConnections();
}

bool TcpServerImpl::updateConnections()
{
	Vec<TcpServerConnection*> dead;

	bool done = false;
	{
		CriticalSectionScope _(m_connCs);
		for(TcpServerConnection* conn : m_connections)
		{
			if(conn->impl()->isDead())
				dead.push_back(conn);
		}
		
		for(TcpServerConnection* conn : dead)
		{
			m_connections.erase(m_connections.find(conn, [](TcpServerConnection* a, TcpServerConnection* b) { return a==b; }));
			m_outer->onDisconnect(conn);
			conn->impl()->closeSocket();
			conn->onDisconnect();
		}

		done = !m_connections.empty();
	}

	return done;
}

void TcpServerImpl::onConnect(TcpServerConnection* connection)
{
	CriticalSectionScope _(m_connCs);
	m_connections.push_back(connection);
	m_outer->onConnect(connection);
}

TcpServer::TcpServer(u16 portNumber, WorkerThreadPool* threadPool, bool secured, u32 readBlockSize)
	: m_portNumber(portNumber)
{
	setReadBlockSize(readBlockSize);
	m_impl = new TcpServerImpl(this, portNumber, threadPool, secured);
}

TcpServer::~TcpServer() = default;
	
u32 TcpServer::getReadBlockSize() const
{
	return m_readBlockSize;
}
	
void TcpServer::setReadBlockSize(u32 readSize)
{
	if(readSize == 0)
		return;
	
	m_readBlockSize = readSize;
}
	
void TcpServer::shutdown()
{
	return m_impl->shutdown();	
}

void TcpServer::update()
{	
	m_impl->update();
}

void TcpServer::forEachConnection(void(*fn)(TcpServerConnection*))
{
	for(TcpServerConnection* conn : m_impl->m_connections)
	{
		fn(conn);
	}
}
}