#pragma once
#include "../Bundle.h"

namespace rw
{
enum eSysMemoryType
{
    MEMTYPE_HOST,           // Allocates using the master allocator
    MEMTYPE_DEVICE,         // Allocates using the system's functions
    MEMTYPE_VIRTUAL,        // smaller allocations
    MEMTYPE_PHYSICAL,       // larger ones
    MEMTYPE_BOOT,

    MEMTYPE_COUNT
};

// sysAllocator::GetCurrent().GetAllocator(MEMTYPE_PHYSICAL)->Allocate()
class sysAllocator
{
public:
    sysAllocator() = default;
    ~sysAllocator() = default;

    static sysAllocator* GetCurrent() {
        return sm_Current;
    }

    static void SetCurrent(sysAllocator* Allocator) {
        sm_Current = Allocator;
    }

    virtual FB_BASE_API sysAllocator* GetAllocator(eSysMemoryType MemoryType);
    virtual FB_BASE_API void SetAllocator(eSysMemoryType MemoryType, sysAllocator* Allocator);

    virtual FB_BASE_API void* Allocate(size_t Size, s32 Align, eSysMemoryType SubType = MEMTYPE_HOST);
    virtual FB_BASE_API size_t QueryAllocationSize(void* Ptr, eSysMemoryType SubType = MEMTYPE_HOST);
    virtual FB_BASE_API int Free(void* Ptr, eSysMemoryType SubType = MEMTYPE_HOST);

private:
    static sysAllocator* sm_Current;
};
}