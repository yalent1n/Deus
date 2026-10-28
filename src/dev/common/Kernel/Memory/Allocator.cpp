#include <Engine.Base/Memory/Allocator.h>

namespace rw
{
sysAllocator* sysAllocator::sm_Current;

sysAllocator* sysAllocator::GetAllocator(eSysMemoryType MemoryType)
{
    (void) MemoryType;
    return NULL;
}

void sysAllocator::SetAllocator(eSysMemoryType MemoryType, sysAllocator* Allocator)
{
}

void* sysAllocator::Allocate(size_t Size, s32 Align, eSysMemoryType SubType)
{
    (void) Size;
    (void) Align;
    (void) SubType;
    return NULL;
}

size_t sysAllocator::QueryAllocationSize(void* Ptr, eSysMemoryType SubType)
{
    (void) Ptr;
    (void) SubType;
    return 0;
}

int sysAllocator::Free(void* Ptr, eSysMemoryType SubType)
{
    (void) Ptr;
    (void) SubType;
    return YES;
}
}