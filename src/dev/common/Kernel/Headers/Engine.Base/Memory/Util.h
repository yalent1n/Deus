#pragma once
#include "Types.h"

#define _ZERO_MEMORY(p, len) rw::memset((p), 0, (len))

namespace rw
{
FB_BASE_API void* memcpy(void* d, const void* s, size_t dlen, const size_t slen);
FB_BASE_API void* memset(void* d, u8 b, const size_t dlen);
}