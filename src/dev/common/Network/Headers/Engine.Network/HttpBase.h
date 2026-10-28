#pragma once
#include <Engine.Base/String/String.h>
#include "http/base.h"

namespace rw
{
FB_NETWORK_API const char* verbName(HttpVerb method);
FB_NETWORK_API HttpVerb methodVerb(const char* method);

// Returns the decoded size.
// @param buffer: String to decode
// @param len: String length
FB_NETWORK_API void urlDecode(void* buffer, u32 len);

struct QueryParameter
{
    const char* key;
    const char* value;
};
FB_NETWORK_API void urlEncode(QueryParameter* params, u32 count, StringEditor& builder);

class QueryStringIterator
{
public:
    FB_NETWORK_API QueryStringIterator(const void* begin, u32 size);
    FB_NETWORK_API ~QueryStringIterator();

    FB_NETWORK_API bool more() const;
    FB_NETWORK_API const QueryParameter& moveNext();
    
private:
    const char* m_cursor;
    const char* m_end;
    QueryParameter m_current;
};
}
