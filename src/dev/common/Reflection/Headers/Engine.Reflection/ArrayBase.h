#pragma once
#include <Engine.Reflection/Bundle.h>

namespace rw
{
namespace array_helper
{
    enum
    {
        kEntrySize = -1,
        kEntryCapacity = -2,
        kEntryCount = 2
    };

    static inline uintptr_t s_emptyArrayData[2] = {0, 0};
};

struct ArrayBase
{
    FB_REFL_API u32 getElementCount(const ArrayBase* base);
    FB_REFL_API void releaseElements(const ArrayBase* base);
};
}