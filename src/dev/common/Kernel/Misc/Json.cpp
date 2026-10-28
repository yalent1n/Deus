#include <Engine.Base/Misc/Json.h>
#include <Engine.Base/Memory/VirtualAlloc.h>
#include <Engine.Base/String/String.h>
#include <cstring>
#include <stdlib.h>

namespace rw
{
json_object::json_object()
	: type(eTypeUndefined)
	, key(nullptr)
	, valuestring(nullptr)
	, child(nullptr)
	, sibling(nullptr)
{}

JsonParser::JsonParser(const void* buf)
	: m_data(static_cast<const char*>(buf))
	, m_len(static_cast<u32>(rw::strlen(m_data)))
{}

JsonParser::JsonParser(const void* buf, u32 len)
	: m_data(static_cast<const char*>(buf))
	, m_len(len)
{}

__forceinline const char* skipWhitespace(const char* s)
{
    while(isspace(*s))
        ++s;

    return s;
}

const char* ReadStringUntil(const char* ptr, char term, u32* keylenout)
{
	if(!ptr)
		return 0;

	u32 keylen = 0;

	const char* cursor = ptr;
	while(cursor[keylen] != term)
		keylen++;

	char* str = (char*) HeapAllocGlobal(keylen+1);
	std::memcpy(str, ptr, keylen);
    str[keylen] = '\0';

	if(keylenout)
		*keylenout = keylen;

	return str;
}

const char* readStringEscaped(const char* ptr, char term, const char** strendout)
{
	if(!ptr)
		return 0;

	StringBuilder<> str;
	while(true)
	{
		char c = *ptr;
		if(c == '\\')
		{
			char ctrl = *++ptr;
			switch(ctrl)
			{
			case '\\':
				str.appendChar('\\');
				break;
			case 'b':
				str.appendChar('\b');
				break;
			case 'f':
				str.appendChar('\f');
				break;
			case 'n':
				str.appendChar('\n');
				break;
			case 'r':
				str.appendChar('\r');
				break;
			case 't':
				str.appendChar('\t');
				break;
			case 'u':
				ptr += 4;
				break;
			}

			continue;
		}
		
		if(c == term)
			break;

		str.appendChar(c);
		ptr++;
	}

	ptr++;
	if(strendout)
		*strendout = ptr;

	return str.detach();
}

const char* json_object::parse(const char* begin, const char* end)
{
	const char* cursor = begin;
	while(cursor < end)
	{
		cursor = skipWhitespace(cursor);
		const char* keyend = nullptr;
		if(cursor[0] == '\"')
			key = readStringEscaped(++cursor, '\"', &keyend);

		if(keyend)
			cursor = keyend;

		cursor = skipWhitespace(cursor);
		if(key) {
			if(cursor[0] == ':')
			{
				cursor = skipWhitespace(++cursor);
			}
			else
			{
				type = eTypeString;
				valuestring = key;
				key = nullptr;
				return cursor;
			}
		}

		bool is_object_begin = cursor[0] == '{';
		bool is_array_begin = cursor[0] == '[';

		if(is_object_begin) {
			type = eTypeObject;
			cursor++;
		}
		else if(is_array_begin) {
			type = eTypeArray;
			cursor++;
		}
		else
			type = eTypeUndefined;

		cursor = skipWhitespace(cursor);

		if(type == eTypeUndefined)
		{
			bool is_number;
			bool is_string;
			bool is_boolean;
			bool is_null;

			is_string = cursor[0] == '\"';
			is_boolean = !compareIgnoreCase("true", cursor, 4) || !compareIgnoreCase("false", cursor, 5);
			is_null = !compareIgnoreCase("null", cursor, 4);
			is_number = isalnum(cursor[0]);

			if(is_number)
			{
				const char* numbegin = cursor;
				const char* numend;
				double d = strtod(numbegin, (char**)&numend);
				int n = int(d);
				bool is_integral = d == n;
				if(is_integral) {
					valueint = n;
					type = eTypeInteger;
				}
				else {
					valuedouble = d;
					type = eTypeDouble;
				}

				return numend;
			}
			if(is_boolean)
			{
				type = eTypeBoolean;
				valueboolean = !(compareIgnoreCase("false", cursor, 4) == 0);
				while(isalchar(*cursor))
					cursor++;
				return cursor;
			}
			if(is_null)
			{
				type = eTypeNull;
				while(isalchar(*cursor))
					cursor++;
				return cursor;
			}
			if(is_string)
			{
				type = eTypeString;
				const char* strend;
				valuestring = readStringEscaped(++cursor, '\"', &strend);
				return strend;
			}
		}
		else
		{
			const char* childcursor = cursor;
			while(childcursor < end)
			{
				json_object* childitem = new json_object();
				childcursor = childitem->parse(childcursor, end);

				if(child)
				{
					json_object* appendto = child;
					while(appendto) {
						if(appendto->sibling==0)
							break;
						appendto = appendto->sibling;
					}
					if(appendto)
						appendto->sibling = childitem;
				}
				else
					child = childitem;

				if(childcursor[0] != ',')
				{
					childcursor = skipWhitespace(childcursor);
					// bool is_object_end = childcursor[0] == '}';
					// bool is_array_end = childcursor[0] == ']';
					//
					// if(is_object_end)
					// 	FB_ASSERT(type == JSON::kObject, "Object terminator is invalid");
					// if(is_array_end)
					// 	FB_ASSERT(type == JSON::kArray, "Array terminator is invalid");
					cursor = ++childcursor;
					break;
				}

				childcursor = skipWhitespace(++childcursor);
			}
		
			return cursor;
		}
	}

	return cursor;
}

json_object* JsonParser::parse() const
{
	const char* begin = m_data;
	const char* end = m_data + m_len;

	if(begin[0] != '{' && begin[0] != '[')
		return nullptr;
	
	json_object* root = new json_object();
	const char* parseend = root->parse(begin, end);
	if(parseend[0] != '\0')
	{
		__builtin_assert(skipWhitespace(parseend)[0] == '\0');
        delete root;
		return nullptr;
	}
	
	return root;
}
}