#include <Engine.Base/Misc/Interlocked.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace rw
{
s16 lock_inc(volatile s16& operand)
{
    return _InterlockedIncrement16((volatile SHORT*) &operand);
}

u16 lock_inc(volatile u16& operand)
{
    return (u16) _InterlockedIncrement16((volatile SHORT*) &operand);
}

s32 lock_inc(volatile s32& operand)
{
    return _InterlockedIncrement((volatile long*) &operand);
}

u32 lock_inc(volatile u32& operand)
{
    return (unsigned) _InterlockedIncrement((volatile long*) &operand);
}

s64 lock_inc(volatile s64& operand)
{
    return _InterlockedIncrement64((volatile LONG64*) &operand);
}

u64 lock_inc(volatile u64& operand)
{
    return (ULONG64) _InterlockedIncrement64((volatile LONG64*) &operand);
}

s16 lock_dec(volatile s16& operand)
{
    return _InterlockedDecrement16((volatile SHORT*) &operand);
}

u16 lock_dec(volatile u16& operand)
{
    return (USHORT) _InterlockedDecrement16((volatile SHORT*) &operand);
}

s32 lock_dec(volatile s32& operand)
{
    return _InterlockedDecrement((volatile long*) &operand);
}

u32 lock_dec(volatile u32& operand)
{
    return (unsigned) _InterlockedDecrement((volatile long*) &operand);
}

s64 lock_dec(volatile s64& operand)
{
    return _InterlockedDecrement64((volatile LONG64*) &operand);
}

u64 lock_dec(volatile u64& operand)
{
    return (ULONG64) _InterlockedDecrement64((volatile LONG64*) &operand);
}

// s16 lock_add(volatile s16& operand, s16 value)
// {
    
// }

// u16 lock_add(volatile u16& operand, u16 value)
// {

// }

s32 lock_add(volatile s32& operand, s32 value)
{
    return (s32) _InlineInterlockedAdd((LONG*) &operand, value);
}

u32 lock_add(volatile u32& operand, u32 value)
{
    return (u32) _InlineInterlockedAdd((LONG*) &operand, (ULONG) value);
}

s64 lock_add(volatile s64& operand, s64 value)
{
    return (s64) _InterlockedExchangeAdd64((LONG64*) &operand, value);
}

u64 lock_add(volatile u64& operand, u64 value)
{
    return (u64) _InterlockedExchangeAdd64((LONG64*) &operand, (LONG64) value);
}

// s16 lock_sub(volatile s16& operand, s16 value)
// {
    
// }

// u16 lock_sub(volatile u16& operand, u16 value)
// {

// }

s32 lock_sub(volatile s32& operand, s32 value)
{
    return _InterlockedExchangeAdd((volatile LONG*) &operand,  - (long) value);
}

u32 lock_sub(volatile u32& operand, u32 value)
{
    return (u32) _InterlockedExchangeAdd((volatile long*) &operand,  - (long) value);
}

s64 lock_sub(volatile s64& operand, s64 value)
{
    return (s64) _InterlockedExchangeAdd64((volatile LONG64*) &operand,  - (LONG64) value);
}

u64 lock_sub(volatile u64& operand, u64 value)
{
    return (u64) _InterlockedExchangeAdd64((volatile LONG64*) &operand,  - (LONG64) value);
}

s16 lock_xchg(volatile s16& operand, s16 value)
{
    return _InterlockedExchange16(&operand, value);
}

u16 lock_xchg(volatile u16& operand, u16 value)
{
    return _InterlockedExchange16((volatile SHORT*) &operand, (SHORT) value);
}

s32 lock_xchg(volatile s32& operand, s32 value)
{
    return InterlockedExchange((volatile LONG*) &operand, (LONG) value);
}

u32 lock_xchg(volatile u32& operand, u32 value)
{
    return (u32) InterlockedExchange((volatile LONG*) &operand, (LONG) value);
}

s64 lock_xchg(volatile s64& operand, s64 value)
{
    return (s64) _InterlockedExchange64((volatile LONG64*) &operand, value);
}

u64 lock_xchg(volatile u64& operand, u64 value)
{
    return (u64) _InterlockedExchange64((volatile LONG64*) &operand, (LONG64) value);
}

s16 lock_cmpxchg(volatile s16& operand, s16 value, s16 cond)
{
    return _InterlockedCompareExchange16((volatile SHORT*) &operand, value, cond);
}

u16 lock_cmpxchg(volatile u16& operand, u16 value, u16 cond)
{
    return _InterlockedCompareExchange16((volatile SHORT*) &operand, (SHORT) value, (SHORT) cond);
}

s32 lock_cmpxchg(volatile s32& operand, s32 value, s32 cond)
{
    return (s32) _InterlockedCompareExchange((volatile LONG*) &operand, value, cond);
}

u32 lock_cmpxchg(volatile u32& operand, u32 value, u32 cond)
{
    return (u32) _InterlockedCompareExchange((volatile LONG*) &operand, (LONG) value, (LONG) cond);
}

s64 lock_cmpxchg(volatile s64& operand, s64 value, s64 cond)
{
    return (s64) _InterlockedCompareExchange64((volatile LONG64*) &operand, value, cond);
}

u64 lock_cmpxchg(volatile u64& operand, u64 value, u64 cond)
{
    return (u64) _InterlockedCompareExchange64((volatile LONG64*) &operand, (LONG64) value, (LONG64) cond);
}
}