#pragma once
#include "../Bundle.h"

namespace rw
{
FB_BASE_API s16 lock_inc(volatile s16&);
FB_BASE_API u16 lock_inc(volatile u16&);
FB_BASE_API s32 lock_inc(volatile s32&);
FB_BASE_API u32 lock_inc(volatile u32&);
FB_BASE_API s64 lock_inc(volatile s64&);
FB_BASE_API u64 lock_inc(volatile u64&);

FB_BASE_API s16 lock_dec(volatile s16&);
FB_BASE_API u16 lock_dec(volatile u16&);
FB_BASE_API s32 lock_dec(volatile s32&);
FB_BASE_API u32 lock_dec(volatile u32&);
FB_BASE_API s64 lock_dec(volatile s64&);
FB_BASE_API u64 lock_dec(volatile u64&);

// FB_BASE_API s16 lock_add(volatile s16&, s16);
// FB_BASE_API u16 lock_add(volatile u16&, u16);
FB_BASE_API s32 lock_add(volatile s32&, s32);
FB_BASE_API u32 lock_add(volatile u32&, u32);
FB_BASE_API s64 lock_add(volatile s64&, s64);
FB_BASE_API u64 lock_add(volatile u64&, u64);

// FB_BASE_API s16 lock_sub(volatile s16&, s16);
// FB_BASE_API u16 lock_sub(volatile u16&, u16);
FB_BASE_API s32 lock_sub(volatile s32&, s32);
FB_BASE_API u32 lock_sub(volatile u32&, u32);
FB_BASE_API s64 lock_sub(volatile s64&, s64);
FB_BASE_API u64 lock_sub(volatile u64&, u64);

FB_BASE_API s16 lock_xchg(volatile s16&, s16);
FB_BASE_API u16 lock_xchg(volatile u16&, u16);
FB_BASE_API s32 lock_xchg(volatile s32&, s32);
FB_BASE_API u32 lock_xchg(volatile u32&, u32);
FB_BASE_API s64 lock_xchg(volatile s64&, s64);
FB_BASE_API u64 lock_xchg(volatile u64&, u64);

FB_BASE_API s16 lock_cmpxchg(volatile s16&, s16, s16 c);
FB_BASE_API u16 lock_cmpxchg(volatile u16&, u16, u16 c);
FB_BASE_API s32 lock_cmpxchg(volatile s32&, s32, s32 c);
FB_BASE_API u32 lock_cmpxchg(volatile u32&, u32, u32 c);
FB_BASE_API s64 lock_cmpxchg(volatile s64&, s64, s64 c);
FB_BASE_API u64 lock_cmpxchg(volatile u64&, u64, u64 c);

/*
    Helper functions for better syntax
*/

template<typename T>
inline T lock_write(volatile T& operand, T value)
    { return lock_xchg(operand, value); }

template<typename T>
inline T lock_read(volatile T& operand)
    { return lock_cmpxchg(operand, 0U, 0U); }
}