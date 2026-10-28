#include <Engine.Reflection/ArrayBase.h>

namespace rw
{
u32 getElementCount(const ArrayBase* base)
{
    return ((u32*)base)[array_helper::kEntryCount];
}

void releaseElements(const ArrayBase* base)
{
    
}
}