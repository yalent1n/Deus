#include <Engine.Network/Relay.h>
#include <Engine.Network/Util.h>

namespace rw
{
u32 ether_atoh(const ether_addr* addr)
{
    if(addr->in.sa_family == AF_INET)
    {
        return DJB(addr, sizeof(sockaddr_in));
    }
    if(addr->in.sa_family == AF_INET6)
    {
        return DJB(addr, sizeof(sockaddr_in6));
    }

    return 0;
}

RelayAgent::RelayAgent()
    : m_socket(winsock::pullup(SOCK_DGRAM, IPPROTO_UDP))
{
    u_long bits = 1;
    int ret = ioctlsocket(m_socket, FIONBIO, &bits);
    EASSERT(ret != SOCKET_ERROR, "ioctlsocket(FIONBIO) failed");
}

RelayAgent::~RelayAgent()
{
    if(m_socket != INVALID_SOCKET)
    {
        Close();
    }
}

bool RelayAgent::Open(u16 port, u32 mask)
{
    SocketAddress addr(mask, port);

    if(bind(m_socket, addr.as<const sockaddr>(), addr.length) == SOCKET_ERROR)
        return false;

    //EDEBUGF("Opened relay at port %u", port);
    return true;
}

bool RelayAgent::Close()
{
    if(m_socket != INVALID_SOCKET)
    {
        //EDEBUGF("Removed relay at %u", m_socket);
        platformClose(m_socket);
        m_socket = INVALID_SOCKET;
    }

    return true;
}

u32 RelayAgent::Read(void* buf, u32 size, ether_addr* from) const
{
    // TODO: allow IPv6
    int addrlen = sizeof(sockaddr_in);

    int len = recvfrom(m_socket, (char*)buf, (int)size, 0, &from->in, &addrlen);
    if(len < 0)
        return 0;
    
    return (u32)len;
}

u32 RelayAgent::Write(const ether_addr* to, const void* buf, u32 size) const
{
    int addrlen = sizeof(sockaddr_in);

    int len = sendto(m_socket, (const char*)buf, (int)size, 0, &to->in, addrlen);
    if(len < 0)
        return 0;

    return (u32)len;
}
}