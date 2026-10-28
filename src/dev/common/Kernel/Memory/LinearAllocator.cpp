#include <Engine.Base/Memory/LinearAllocator.h>
#include <Engine.Base/Memory/VirtualAlloc.h>

namespace rw
{
sysLinearAllocator::sysLinearAllocator(size_t HeapSize)
{
    DWORD Res = CreateHeapObject(&mp_HeapBase, HeapSize);
    EASSERTF(Res, "Failed to create linear allocator with Heap Size=%zu", HeapSize);
}

sysLinearAllocator::~sysLinearAllocator()
{
    DWORD Res = DestroyHeapObject(&mp_HeapBase);
    EASSERT(Res, "Failed to destroy linear allocator");
}

void* sysLinearAllocator::Allocate(size_t Size, s32 Align, eSysMemoryType SubType)
{
    (void) SubType;
    return HeapAlloc(&mp_HeapBase, Size, Align);
}

size_t sysLinearAllocator::QueryAllocationSize(void* Ptr, eSysMemoryType SubType)
{
    (void) SubType;
    return HeapQuerySize(&mp_HeapBase, Ptr);
}

int sysLinearAllocator::Free(void* Ptr, eSysMemoryType SubType)
{
    (void) SubType;
    return HeapFree(&mp_HeapBase, Ptr) == Ptr;
}
}