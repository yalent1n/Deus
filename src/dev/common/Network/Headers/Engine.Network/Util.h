#pragma once
#include "SocketBase.h"
#include <Engine.Base/String/String.h>

namespace rw
{
// Converts hostname to IP address (DNS resolution, IPv4)
FB_NETWORK_API bool resolveDns(const char *host, void *destData);

// Resolve host and set port in address given in the parameter.
// @param hostname: Identifier of the host (Possible values contains; localhost, "", DOT-SEPARATED, DOMAIN NAME)
// @param port: Port to bind to
// @param address: This will receive the resolved data from the given parameters
FB_NETWORK_API bool resolveHostWithPort(const char *hostname, u16 port, SocketAddress& address);

// Resolve host and set port in address given in the parameter.
// @param hostnameAndPort: Combination of identifier of the host and a port 
// (Valid values are: localhost, "", dotted ip, domain name, these should be followed by a colon and the port)
// @param address: This will receive the resolved data from the given parameter
FB_NETWORK_API bool resolveHostWithPort(const char *hostnameAndPort, SocketAddress& address);

// Converts a socket address to a string (IPv4/6 representation)
FB_NETWORK_API bool inet_aton(const sockaddr* addr, StringEditor& result);

// Converts a netmask to a length prefix (IPv4/6 representation)
FB_NETWORK_API bool inet_ntol(const sockaddr* addr, StringEditor& result);

// Converts the socket's address to a string (IPv4/6 representation)
// @param socket: Socket that holds the address
// @param str: String that will have the address
FB_NETWORK_API bool socketToString(u64 socket, StringEditor& str);
}