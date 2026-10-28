#pragma once
#include "SocketBase.h"
#include <ws2ipdef.h>

namespace rw
{
union ether_addr
{
    sockaddr in;
    sockaddr_in in4;
    sockaddr_in6 in6;
};

FB_NETWORK_API u32 ether_atoh(const ether_addr*);

typedef u32 dl_addr_t;

// An UDP socket implementation.
class RelayAgent
{
public:
    FB_NETWORK_API RelayAgent();
    FB_NETWORK_API ~RelayAgent();

    SOCKET Socket() const
    {
        return m_socket;
    }

    FB_NETWORK_API bool Open(u16 port, u32 mask = 0);
    FB_NETWORK_API bool Close();

    FB_NETWORK_API u32 Read(void* buf, u32 size, ether_addr* from) const;
    FB_NETWORK_API u32 Write(const ether_addr* to, const void* buf, u32 size) const;

private:
    SOCKET m_socket;
};
}