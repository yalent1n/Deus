#pragma once
#include "Allocator.h"

namespace rw
{
class sysHeapAllocator : public sysAllocator
{
public:
    FB_BASE_API sysHeapAllocator();
    FB_BASE_API ~sysHeapAllocator();

    FB_BASE_API sysAllocator* GetAllocator(eSysMemoryType MemoryType) override;
    FB_BASE_API void SetAllocator(eSysMemoryType MemoryType, sysAllocator* Allocator) override;
    FB_BASE_API void* Allocate(size_t Size, s32 Align, eSysMemoryType SubType = MEMTYPE_HOST) override;
    FB_BASE_API size_t QueryAllocationSize(void* Ptr, eSysMemoryType SubType = MEMTYPE_HOST) override;
    FB_BASE_API int Free(void* Ptr, eSysMemoryType SubType = MEMTYPE_HOST) override;

private:
    sysAllocator* m_Allocators[MEMTYPE_COUNT];
};

struct sysHeapBootConfig {
    eSysMemoryType          HostArenaType;
    eSysMemoryType          DeviceArenaType;

    eSysMemoryType          VirtualArenaType;
    eSysMemoryType          PhysicalArenaType;

    eSysMemoryType          BootArenaType;

    size_t                  HostArenaSize;
    size_t                  DeviceArenaSize;

    size_t                  VirtualArenaSize;
    size_t                  PhysicalArenaSize;

    size_t                  BootArenaSize;
};

FB_BASE_API sysHeapBootConfig* sysLoadHeapBootConfig();

FB_BASE_API sysResult sysInitializeHeapAllocator();
FB_BASE_API sysResult sysDeInitializeHeapAllocator();
}