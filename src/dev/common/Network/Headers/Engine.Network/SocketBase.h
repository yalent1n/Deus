#pragma once
#include "Bundle.h"

namespace rw
{
enum class ConnectionState
{
	Active,					// connection is active, can be read/write
	Terminated,				// connection was terminated by the other peer
	Disconnected,			// connection was disconnected by the other peer
	Pending,				// connection has pending requests
	Dead					// connection is has been closed completely
};
	
enum ConnectionReadBlockSize : u32
{
	ConnectionReadBlockSize_Quick = 1024,					// frequent updates, smaller packets, like a game connection
	ConnectionReadBlockSize_Normal = 2048,					// occasional, not that large packets, like a http connection
	ConnectionReadBlockSize_Large = 8096,					// known to receive large packets, like a file service connection
	ConnectionReadBlockSize_ExtraLarge = 65536				// why...
};

enum class IoState
{
	Open,
	Flight,
	Completed,
	Failed,
	Cancelled,
	Terminated,
	Closed
};

// Returns WSAGetLastError()
LIB_LINKAGE int XNetGetErrorCode();

// Returns the error text for the _sockLastError()_ code
LIB_LINKAGE const char *XNetGetErrorMessage(int err);

static inline const char* XNetGetErrorMessageLast()
{
	return XNetGetErrorMessage(XNetGetErrorCode());
}

// Translation between native sockaddr/addrinfo using constructors for hostname/port resolution.
class SocketAddress
{
public:
	enum {

		MaxNativeDataOctets = 16, // sizeof(sockaddr)
	};

	SocketAddress() = default;
	
	// Construct from native data.
	FB_NETWORK_API SocketAddress(const void *data, size_t length);

	// Construct from an IPv4 address.
	FB_NETWORK_API SocketAddress(u32 addr, u16 port);

	// Construct from a DNS name and port.
	FB_NETWORK_API SocketAddress(const char *host, u16 port);

	// Construct from a DNS name and port combination separated by a colon.
	FB_NETWORK_API SocketAddress(const char *hostAndPort);

	// Copy _length_ bytes from _data_ to this instance's _nativeData_.
	// @param data: Data to copy
	// @param length: Length of native data
	void SetData(const void *data, u8 length);

	// Clear the current address data.
	void Clear();

	u32 GetInetAddress() const;
	u16 GetInetPort() const;

	// Returns the data cast to the given type.
	template<typename T> T *as() const { return (T*)nativeData; }

	u8 nativeData[MaxNativeDataOctets] {0};
	int length {0};

	inline bool operator==(const SocketAddress &addr) const
	{				
		return ((const u64 *)this)[0] == ((const u64 *)&addr)[0] 
			&& ((const u64 *)this)[1] == ((const u64 *)&addr)[1] 
			&& length == addr.length; // tricky as fuck, but ig it's faster to compare numbers instead of memcmp.
	}
};

struct SocketAddressHash
{
	unsigned operator()(const SocketAddress& addr) const;
};

extern int platformConnect(int sock, const SocketAddress& addr);
extern int platformClose(int sock);
}