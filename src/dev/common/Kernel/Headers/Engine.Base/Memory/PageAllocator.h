#pragma once
#include "Allocator.h"
#include "../HashMap.h"

namespace rw
{
class sysPageAllocator : public sysAllocator
{
public:
    FB_BASE_API sysPageAllocator(size_t PrimarySize = NULL, bool PrimaryCanExecuteCode = NO);
    FB_BASE_API ~sysPageAllocator();

    FB_BASE_API void* Allocate(size_t Size, s32 Align, eSysMemoryType SubType = MEMTYPE_HOST) override;
    FB_BASE_API size_t QueryAllocationSize(void* Ptr, eSysMemoryType SubType = MEMTYPE_HOST) override;
    FB_BASE_API int Free(void* Ptr, eSysMemoryType SubType = MEMTYPE_HOST) override;

private:
    eastl::hash_map<void*, size_t> m_RegionSizeCache;
};
}