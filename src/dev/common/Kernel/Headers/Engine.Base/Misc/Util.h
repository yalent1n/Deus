#pragma once
#include "../Types.h"

namespace rw
{
// Variable-length integer methods

LIB_LINKAGE u32 decodeSize(const void*);

LIB_LINKAGE u32 decodeVarUint(const void*);
LIB_LINKAGE u64 decodeVarUint64(const void*);
LIB_LINKAGE s32 decodeVarInt(const void*);
LIB_LINKAGE s64 decodeVarInt64(const void*);
}