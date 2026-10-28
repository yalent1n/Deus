#include <Engine.Network/SocketBase.h>
#include <Engine.Network/Util.h>
#include <WS2tcpip.h>

namespace rw
{
int XNetGetErrorCode()
{
	return WSAGetLastError();
}

SocketAddress::SocketAddress(const void *data, size_t length)
{
	SetData(data, u8(length));
}
SocketAddress::SocketAddress(u32 addr, u16 port)
	: nativeData()
	, length(MaxNativeDataOctets)
{
	as<sockaddr>()->sa_family = AF_INET;
	as<sockaddr_in>()->sin_addr.S_un.S_addr = htonl(addr);
	as<sockaddr_in>()->sin_port = htons(port);
}
SocketAddress::SocketAddress(const char *host, u16 port)
	: nativeData()
	, length(MaxNativeDataOctets)
{		
	__builtin_assert(resolveHostWithPort(host, port, *this));
}
SocketAddress::SocketAddress(const char *hostAndPort)
	: nativeData()
	, length(MaxNativeDataOctets)
{
	__builtin_assert(resolveHostWithPort(hostAndPort, *this));
}

void SocketAddress::SetData(const void *data, u8 length)
{
	EASSERT(data, "native data shall not be null");
	::memcpy_s(nativeData, MaxNativeDataOctets, data, length);
	this->length = length;
}

void SocketAddress::Clear()
{
	memset(nativeData, 0, MaxNativeDataOctets);
	length = 0;
}

u32 SocketAddress::GetInetAddress() const
{
	const sockaddr_in* sockaddr = as<const sockaddr_in>();
	//__builtin_assert(sockaddr->sin_family == AF_INET);
	return sockaddr->sin_addr.S_un.S_addr;
}
u16 SocketAddress::GetInetPort() const
{
	const sockaddr_in* sockaddr = as<const sockaddr_in>();
	//__builtin_assert(sockaddr->sin_family == AF_INET);
	return htons(sockaddr->sin_port);
}

unsigned SocketAddressHash::operator()(const SocketAddress& addr) const
{
	if(addr.length == 0)
		return 0;

	return static_cast<unsigned>(addr.as<const sockaddr_in>()->sin_addr.S_un.S_addr);
	//return eastl::djb_hash(addr.nativeData, addr.length);
}
	
int platformConnect(int sock, const SocketAddress& addr)
{
	return connect(sock, addr.as<const sockaddr>(), addr.length);
}

int platformClose(int sock)
{
	shutdown(sock, SD_BOTH);
	return closesocket(sock);
}

const char *XNetGetErrorMessage(int err)
{
	switch(err)
	{	
	case WSAEINTR:
		return "A blocking operation was interrupted by a call to WSACancelBlockingCall";
	case WSAEFAULT:
		return "The system detected an invalid pointer address in attempting to use a pointer argument in a call";
	case WSAEINVAL:
		return "An invalid argument was supplied";
	case WSAEMFILE:
		return "Too many open sockets";
	case WSAEWOULDBLOCK:
		return "A non-blocking socket operation could not be completed immediately";
	case WSAEINPROGRESS:
		return "A blocking operation is currently executing";
	case WSAEALREADY:
		return "An operation was attempted on a non-blocking socket that already had an operation in progress";
	case WSAENOTSOCK:
		return "An operation was attempted on something that is not a socket";
	case WSAEDESTADDRREQ:
		return "A required address was omitted from an operation on a socket";
	case WSAEMSGSIZE:
		return "A message sent on a datagram socket was larger than the internal message buffer or some other network limit, or the buffer used to receive a datagram into was smaller than the datagram itself";
	case WSAEADDRINUSE:
		return "Only one usage of each socket address (protocol/network address/port) is normally permitted";
	case WSAEADDRNOTAVAIL:
		return "The requested address is not valid in its context";
	case WSAENETDOWN:
		return "A socket operation encountered a dead network";
	case WSAENETUNREACH:
		return "A socket operation was attempted to an unreachable network";
	case WSAENETRESET:
		return "The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress";
	case WSAECONNABORTED:
		return "An established connection was aborted by the software in your host machine";
	case WSAECONNRESET:
		return "An existing connection was forcibly closed by the remote host";
	default:
		return "Undefined";
	}
}
}