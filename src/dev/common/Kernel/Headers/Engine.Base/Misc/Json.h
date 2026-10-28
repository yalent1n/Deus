#pragma once
#include "../Bundle.h"

namespace rw
{
struct json_object
{
	FB_BASE_API json_object();

	enum TypeBits : u32
	{
		eTypeUndefined,
		eTypeObject,
		eTypeArray,
		eTypeString,
		eTypeInteger,
		eTypeDouble,
		eTypeBoolean,
		eTypeNull
	};
	
	TypeBits type;
	const char* key;

	union {
		const char* valuestring;
		long long valueint;
		double valuedouble;
		bool valueboolean;
	};
	
	json_object* child;
	json_object* sibling;
	
	FB_BASE_API const char* parse(const char* begin, const char* end);
};


class JsonParser
{
public:
	FB_BASE_API JsonParser(const void* buf);
	FB_BASE_API JsonParser(const void* buf, u32 len);

	FB_BASE_API json_object* parse() const;

private:
	const char* m_data;
	u32 m_len;
};
}