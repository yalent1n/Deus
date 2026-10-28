#include <Engine.Base/Crypto/Guid.h>
#include <Engine.Base/String/String.h>
#include <cstring>
#include <stdio.h>

#ifdef SvcCompilerMSVC
#define WIN32_LEAN_AND_MEAN
#include <combaseapi.h>
#pragma comment(lib, "Ole32.lib")
#endif

namespace rw
{
u64 Guid::low() const { return reinterpret_cast<const u64*>(this)[0]; }
u64 Guid::high() const { return reinterpret_cast<const u64*>(this)[1]; }

bool Guid::operator==(const Guid& rhs) const
{
    return low() == rhs.low() && high() == rhs.high();			
}
bool Guid::operator!=(const Guid& other) const
{
    return !(*this == other);			
}
int Guid::operator<(const Guid& other) const
{
    return std::memcmp((void*) this, &other, sizeof(Guid));
}

const Guid& Guid::operator=(const Guid& rhs)
{
    std::memcpy((void*) this, &rhs, sizeof(Guid));
    return *this;
}

Guid::Guid(const u8* data)
{
	if (!data)
		return;

	data1 = *(u32*)data;
	data2 = *(u16*)(data + 4);
	data3 = *(u16*)(data + 6);
	data4 = *(u16*)(data + 8);
	data5[0] = data[10];
	data5[1] = data[11];
	data5[2] = data[12];
	data5[3] = data[13];
	data5[4] = data[14];
	data5[5] = data[15];
}

Guid::Guid(const Guid& other)
	: data1(other.data1)
	, data2(other.data2)
	, data3(other.data3)
	, data4(other.data4)
{
	std::memcpy(data5, other.data5, sizeof(data5));
}

Guid::~Guid()
{
}

Guid getMd5Guid(const void* ptr, size_t size)
{
	Guid g;

	// EA::Crypto::MD5::MD5Context ctx;
	// MD5Initialize(ctx);
	// MD5Update(ctx, ptr, size);
	// MD5Finalize(ctx, &g);
	return g;
}

Guid genFastGuid()
{
	Guid guid;
	(void) CoCreateGuid(reinterpret_cast<GUID*>(&guid));
	return guid;
}	

GuidStr::GuidStr(const Guid& guid)
{
	snprintf(str, sizeof str, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		guid.data1,
		guid.data2,
		guid.data3,
		guid.data4 & 0x00FF,
	    (guid.data4 & 0xFF00) >> 8,
		guid.data5[0], guid.data5[1], guid.data5[2], guid.data5[3], guid.data5[4], guid.data5[5]);
}

GuidStrU::GuidStrU(const Guid& guid)
{
	snprintf(str, sizeof str, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		guid.data1,
		guid.data2,
		guid.data3,
		guid.data4 & 0x00FF,
	    (guid.data4 & 0xFF00) >> 8,
		guid.data5[0], guid.data5[1], guid.data5[2], guid.data5[3], guid.data5[4], guid.data5[5]);
	// FixedStringBuilder<BUFFER_LENGTH> tmp; 
	// toHexString(&guid.data1, sizeof(guid.data1), tmp, false);
	// tmp.append('-');
	// toHexString(&guid.data2, sizeof(guid.data2), tmp, false);
	// tmp.append('-');
	// toHexString(&guid.data3, sizeof(guid.data3), tmp, false);
	// tmp.append('-');
	// u8 lo = guid.data4 & 0x00FF;
	// u8 hi = (guid.data4 & 0xFF00) >> 8;
	// toHexString(&lo, 1, tmp, false);
	// toHexString(&hi, 1, tmp, false);
	// tmp.append('-');
	// for(u8 i = 0; i < 6; i++)
	// 	toHexString(guid.data5 + i, 1, tmp, false);
		
	// IOCopyMemory(tmp.data(), BUFFER_LENGTH, str, BUFFER_LENGTH);
}

bool Guid::parse(const char* str)
{
	size_t len = rw::strlen(str);
	if(len != 36)
		return false;
	
	fromhexstr(str, len, this, sizeof(Guid));
	// data1 = endianSwap(data1);
	// data2 = endianSwap(data2);
	// data3 = endianSwap(data3);
	// data4 = endianSwap(data4);
	return true;
}

bool Guid::parse(const char* str, size_t len)
{
	if(len != 36)
		return false;
	
	fromhexstr(str, len, this, sizeof(Guid));
	// data1 = endianSwap(data1);
	// data2 = endianSwap(data2);
	// data3 = endianSwap(data3);
	// data4 = endianSwap(data4);
	return true;
}
}