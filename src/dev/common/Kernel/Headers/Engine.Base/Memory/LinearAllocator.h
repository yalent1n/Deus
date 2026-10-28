#pragma once
#include "Allocator.h"

namespace rw
{
class sysLinearAllocator : public sysAllocator
{
public:
    FB_BASE_API sysLinearAllocator(size_t HeapSize);
    FB_BASE_API ~sysLinearAllocator();

    FB_BASE_API void* Allocate(size_t Size, s32 Align, eSysMemoryType SubType = MEMTYPE_HOST) override;
    FB_BASE_API size_t QueryAllocationSize(void* Ptr, eSysMemoryType SubType = MEMTYPE_HOST) override;
    FB_BASE_API int Free(void* Ptr, eSysMemoryType SubType = MEMTYPE_HOST) override;

private:
    void* mp_HeapBase;
    void* mp_HeapCursor;
    void* m_HeapAllocations;
};
}