#include <Engine.Network/Util.h>
#include <WS2tcpip.h>

#include <Engine.Base/String/String.h>

#pragma comment(lib, "ws2_32.lib")
namespace rw
{
bool resolveDns(const char *host, void *destData)
{
	addrinfo hints, *ai;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	//hints.ai_socktype = SOCK_STREAM;

	if (int err = getaddrinfo(host, nullptr, &hints, &ai))
	{
		char * str = gai_strerrorA(err);
		EERRORF("%s", str);
		return false;
	}

	sockaddr_in *sa = (sockaddr_in *)ai->ai_addr;
	memcpy(destData, &sa->sin_addr.S_un.S_addr, sizeof(in_addr));	
	freeaddrinfo(ai);
	return true;
}
bool resolveHostWithPort(const char *hostname, u16 port, SocketAddress &address)
{
	sockaddr_in sa;
	memset(&sa, 0, sizeof sa);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);

	// default to 0
	if(isnil(hostname))
	{
		sa.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	// default to 127.0.0.1
	else if(equalsIgnoreCase("localhost", hostname))
	{
		sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	}
	// convert dot-separated IP to long (like 192.168.0.1)
	else if(inet_pton(AF_INET, hostname, &sa.sin_addr.s_addr))
	{
	}
	// converts a domain name to an ip (like youtube.com)
	else if(resolveDns(hostname, &sa.sin_addr.s_addr))
	{		
	}
	else
	{
		EERRORF("Failed to resolve %s", hostname);
		return false;
	}

	address.SetData(&sa, sizeof sa);
	address.length = sizeof(sockaddr_in);
	return true;
}

bool resolveHostWithPort(const char *hostnameAndPort, SocketAddress &address)
{
	FixedStringBuilder<256> hostname;
	const char *host = nullptr;
	u16 port = 0;

	int colon = indexOf(':', hostnameAndPort);
	if(colon >= 0)
	{
		if(colon) {
			hostname.appendChars(hostnameAndPort, colon);
			host = c_str(hostname);
		}
		
		port = atoi(hostnameAndPort + colon + 1);
		return resolveHostWithPort(host, port, address);
	}

	EERROR("malformed hostnameAndPort (it should be {hostname}:{port})");
	return false;
}

bool ipv4_is_netmask(uint32_t mask) 
{
    uint32_t m = ntohl(mask);
    uint32_t inv = ~m;
    return (inv & (inv + 1)) == 0;
}

bool ipv6_is_netmask(const u8* mask) {
    bool zeroSeen = false;

    for (int i = 0; i < 16; i++) {
        u8 byte = mask[i];

        for (int b = 7; b >= 0; b--) {
            bool bit = byte & (1 << b);

            if (!bit) {
                zeroSeen = true;
            } else if (zeroSeen) {
                // 1 after a 0 → invalid
                return false;
            }
        }
    }
    return true;
}

int ipv4_mask_to_pfx(const u8* mask) 
{
    int count = 0;
    for (int i = 0; i < 4; i++) {
        u8 byte = mask[i];
        for (int b = 7; b >= 0; b--) {
            if (byte & (1 << b)) count++;
            else return count;
        }
    }
    return count;
}

int ipv6_mask_to_pfx(const u8* mask) 
{
    int count = 0;
    for (int i = 0; i < 16; i++) {
        u8 byte = mask[i];
        for (int b = 7; b >= 0; b--) {
            if (byte & (1 << b)) count++;
            else return count;
        }
    }
    return count;
}

bool inet_aton(const sockaddr* addr, StringEditor& result)
{
	if(addr->sa_family == AF_INET)
	{
		const sockaddr_in *sa = (const sockaddr_in*) addr;
		
		// have to be u8, otherwise it will append the byte value, we want the string value
		const u8 *octs = (const u8*)&sa->sin_addr;
		tostring(octs[0], result); result << '.';
		tostring(octs[1], result); result << '.';
		tostring(octs[2], result); result << '.';
		tostring(octs[3], result);
		
		u16 port = htons(sa->sin_port);
		if(port)
		{
			result.appendChar(':');
			tostring(port, result);
		}
		return true;
	}
	else if(addr->sa_family == AF_INET6)
	{
		const sockaddr_in6 *sa6 = (const sockaddr_in6*) addr;

		const u8* octs = sa6->sin6_addr.u.Byte;

		if(memcmp(octs, &c_emptyGuid, 14) == 0 && (*(u16*)(octs + 15)) == 1)
		{
			result << "::1";
			return true;
		}

		tohexstr(octs + 0, 2, result); result << ':';
		if(!sa6->sin6_scope_id)
		{
			tohexstr(octs + 2, 2, result); result << ':';
			tohexstr(octs + 4, 2, result); result << ':';
			tohexstr(octs + 6, 2, result);
		}

		if((*(u64*)(octs + 8)))
		{
			result << ':';
			tohexstr(octs + 8, 2, result); result << ':';
			tohexstr(octs + 10, 2, result); result << ':';
			tohexstr(octs + 12, 2, result); result << ':';
			tohexstr(octs + 14, 2, result);
		}

		return true;
	}

	return false;
}

bool inet_ntol(const sockaddr* addr, StringEditor& result)
{
	const sockaddr_in *sa = (const sockaddr_in*) addr;
	const sockaddr_in6 *sa6 = (const sockaddr_in6*) addr;
	
	if(addr->sa_family == AF_INET)
	{
		if(ipv4_is_netmask(sa->sin_addr.S_un.S_addr))
		{
			int count = ipv4_mask_to_pfx((const u8*)&sa->sin_addr.S_un.S_addr);
			result << '/';
			tostring(count, result);
		}
		return true;
	}
	else if(addr->sa_family == AF_INET6)
	{
		if(ipv6_is_netmask(sa6->sin6_addr.u.Byte))
		{
			int count = ipv6_mask_to_pfx(sa6->sin6_addr.u.Byte);
			result << '/';
			tostring(count, result);
		}
		return true;
	}

	return false;
}

bool socketToString(u64 socket, StringEditor &str)
{
	if(socket == -1)
		return false;

	sockaddr_in sa;
	int salen = sizeof sa;

	getpeername(socket, (sockaddr*)&sa, &salen);

	const u8 *octs = (const u8*)&sa.sin_addr.S_un.S_addr;
	str << octs[0] << '.'
		<< octs[1] << '.'
		<< octs[2] << '.'
		<< octs[3];

	str << ':' << htons(sa.sin_port);
	return true;
}
}