#pragma once
#include "../Bundle.h"

namespace rw
{
typedef void* EventHandle;

inline constexpr u32 TimeoutImmediate = 0;

enum class EventState : u8
{
    Set,
    Reset,
    Error
};

class Event
{
public:
    FB_BASE_API      Event(bool manualReset = true, bool initialState = false, const char* name = NIL);
    FB_BASE_API     ~Event();

    FB_BASE_API bool set() const;
    FB_BASE_API bool reset() const;
    FB_BASE_API EventState testState(u32 waitMs = TimeoutImmediate) const;

private:
    EventHandle m_eventHandle;
};
}