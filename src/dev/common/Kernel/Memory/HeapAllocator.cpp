#include <Engine.Base/Memory/HeapAllocator.h>
#include <Engine.Base/Memory/LinearAllocator.h>
#include <Engine.Base/Memory/PageAllocator.h>

namespace rw
{
sysHeapAllocator::sysHeapAllocator()
{
}

sysHeapAllocator::~sysHeapAllocator()
{
}

sysAllocator* sysHeapAllocator::GetAllocator(eSysMemoryType MemoryType)
{
    __builtin_assert(MemoryType < MEMTYPE_COUNT);
    return m_Allocators[int(MemoryType)];
}

void sysHeapAllocator::SetAllocator(eSysMemoryType MemoryType, sysAllocator* Allocator)
{
    __builtin_assert(MemoryType < MEMTYPE_COUNT);
    m_Allocators[int(MemoryType)] = Allocator;
}

void* sysHeapAllocator::Allocate(size_t Size, s32 Align, eSysMemoryType SubType)
{
    __builtin_assert(SubType < MEMTYPE_COUNT);
    sysAllocator* Alc = m_Allocators[int(SubType)];
    if(!Alc)
    {
        return NULL;
    }

    return Alc->Allocate(Size, Align);
}

size_t sysHeapAllocator::QueryAllocationSize(void* Ptr, eSysMemoryType SubType)
{
    __builtin_assert(SubType < MEMTYPE_COUNT);
    sysAllocator* Alc = m_Allocators[int(SubType)];
    if(!Alc)
    {
        return NULL;
    }

    return Alc->QueryAllocationSize(Ptr);
}

int sysHeapAllocator::Free(void* Ptr, eSysMemoryType SubType)
{
    __builtin_assert(SubType < MEMTYPE_COUNT);
    sysAllocator* Alc = m_Allocators[int(SubType)];
    if(!Alc)
    {
        return NULL;
    }

    return Alc->Free(Ptr);
}

__forceinline sysAllocator* sysCreateAllocator(eSysMemoryType MemoryType, size_t Size)
{
    switch(MemoryType)
    {
        case MEMTYPE_HOST:
            return new sysHeapAllocator();
        case MEMTYPE_VIRTUAL:
            return new sysLinearAllocator(Size);
            case MEMTYPE_DEVICE:
            return new sysPageAllocator(Size);
        case MEMTYPE_PHYSICAL:
            return new sysPageAllocator(Size);
        case MEMTYPE_BOOT:
            return new sysLinearAllocator(Size);
        default:
            return NULL;
    }
}

sysHeapBootConfig* sysLoadHeapBootConfig()
{
    static sysHeapBootConfig s_BootConfig = 
    {
        MEMTYPE_HOST, MEMTYPE_DEVICE, MEMTYPE_VIRTUAL, MEMTYPE_PHYSICAL, MEMTYPE_BOOT,
        NULL,         96 MiB,         512 MiB,         12 MiB,           16 MiB
    };

    return &s_BootConfig;
}

sysResult sysInitializeHeapAllocator()
{
    if(sysAllocator::GetCurrent())
    {
        return YES;
    }
    
    sysHeapBootConfig* BootConfig = sysLoadHeapBootConfig();
    if(!BootConfig)
    {
        return NO;
    }

    sysAllocator* Master = sysCreateAllocator(MEMTYPE_HOST, 0);
    if(!Master)
    {
        return NO;
    }

    // Configure the heap allocator to own those allocators.
    for(s32 i = MEMTYPE_DEVICE; i < MEMTYPE_COUNT; i++)
    {
        eSysMemoryType MemoryType = (&BootConfig->HostArenaType)[i];
        size_t Size = (&BootConfig->HostArenaSize)[i];

        sysAllocator* Allocator = sysCreateAllocator(MemoryType, Size);
        if(!Allocator)
        {
            return NO;
        }

        Master->SetAllocator(MemoryType, Allocator);
    }

    sysAllocator::SetCurrent(Master);
    return YES;
}

sysResult sysDeInitializeHeapAllocator()
{
    sysAllocator* masterAllocator = sysAllocator::GetCurrent();
    if(!masterAllocator)
    {
        return YES;
    }

    // Remove the trick.
    masterAllocator->SetAllocator(MEMTYPE_HOST, NULL);

    // Free allocators from system heap.
    for (s32 i = MEMTYPE_DEVICE; i < MEMTYPE_COUNT; i++)
    {
        sysAllocator* Allocator = masterAllocator->GetAllocator((eSysMemoryType) i);
        delete Allocator;
    }

    // Destroy actual master
    delete masterAllocator;
    sysAllocator::SetCurrent(NULL);
    return YES;
}
}