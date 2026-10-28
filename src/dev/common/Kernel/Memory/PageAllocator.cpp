#include <Engine.Base/Memory/PageAllocator.h>
#include <Engine.Base/Memory/Map.h>

namespace rw
{
sysPageAllocator::sysPageAllocator(size_t Size, bool CanExecuteCode)
{
    void* FirstBasePtr = sysMem.Allocate(Size, CanExecuteCode);
    m_RegionSizeCache.insert(FirstBasePtr, Size);
}

sysPageAllocator::~sysPageAllocator()
{
}

void* sysPageAllocator::Allocate(size_t Size, s32 Align, eSysMemoryType SubType)
{
    (void) Align;

    void* BasePtr = sysMem.Allocate(Size);
    m_RegionSizeCache.insert(BasePtr, Size);
    return BasePtr;
}

size_t sysPageAllocator::QueryAllocationSize(void* Ptr, eSysMemoryType SubType)
{
    auto i = m_RegionSizeCache.find(Ptr);
    if(i != m_RegionSizeCache.end())
    {
        return i->second;
    }
    
    return NULL;
}

int sysPageAllocator::Free(void* Ptr, eSysMemoryType SubType)
{
    auto i = m_RegionSizeCache.find(Ptr);
    if(i != m_RegionSizeCache.end())
    {
        m_RegionSizeCache.erase(i);
        return YES;
    }
    
    return NO;
}
}