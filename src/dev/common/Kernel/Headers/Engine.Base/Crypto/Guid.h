#pragma once
#include "../Bundle.h"

namespace rw
{
struct GuidStr;

struct Guid
{
	u32 data1 = 0; u16 data2 = 0, data3 = 0, data4 = 0; u8 data5[6] = { 0 };

	FB_BASE_API Guid() = default;
	FB_BASE_API Guid(const u8* data);
	FB_BASE_API Guid(const Guid& other);
	FB_BASE_API ~Guid();

	FB_BASE_API bool parse(const char* str);
	FB_BASE_API bool parse(const char* str, size_t len);
	
	FB_BASE_API u64 low() const;
	FB_BASE_API u64 high() const;

	FB_BASE_API bool operator==(const Guid& rhs) const;
	FB_BASE_API bool operator!=(const Guid& other) const;	
	FB_BASE_API int operator<(const Guid& other) const;	
	FB_BASE_API const Guid& operator=(const Guid& rhs);	
};
	
static inline const Guid c_emptyGuid { };

// lowercase-variant
struct GuidStr
{
	static inline const u32 BUFFER_LENGTH = 37;
	char str[BUFFER_LENGTH];

	FB_BASE_API GuidStr() = default;
	FB_BASE_API GuidStr(const Guid& guid);
};

// uppercase-variant
struct GuidStrU
{
	static inline const u32 BUFFER_LENGTH = 37;
	char str[BUFFER_LENGTH];

	FB_BASE_API GuidStrU() = default;
	FB_BASE_API GuidStrU(const Guid& guid);
};

extern FB_BASE_API Guid getMd5Guid(const void* ptr, size_t size);
extern FB_BASE_API Guid genFastGuid();
}
