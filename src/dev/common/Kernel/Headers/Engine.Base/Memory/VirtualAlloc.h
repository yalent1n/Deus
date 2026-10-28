#pragma once
#include "Types.h"

namespace rw
{
// Maximum amount of space where we can do anything.
inline constexpr size_t kVirtualSpaceMax = 512 MiB;
inline constexpr size_t HEAP_OBJECT_LENGTH = 24;

typedef void* HHEAP;

#ifndef DWORD
    typedef unsigned long DWORD;
#endif

#ifdef ENGINE_HAS_GLOBAL_HEAP
LIB_LINKAGE EXTERN_C int        InitGlobalHeap ();
LIB_LINKAGE EXTERN_C int        ShutdownGlobalHeap ();
LIB_LINKAGE EXTERN_C HHEAP      ObtainGlobalHeapHandle ();
#else
#define ENGINE_USE_ALLOCATOR 1 
#endif

LIB_LINKAGE DWORD               CreateHeapObject (HHEAP _Heap, size_t _VirtualSpace);
LIB_LINKAGE DWORD               DestroyHeapObject (HHEAP _Heap);

LIB_LINKAGE void*               HeapAlloc (HHEAP heap, size_t _AllocationSize, u32 _AllocationAlignment = 16);
LIB_LINKAGE size_t              HeapQuerySize (HHEAP heap, void* _Ptr);
LIB_LINKAGE void*               HeapRealloc (HHEAP heap, void* _Ptr, size_t _NewSize, u32 _NewAlignment = 16);
LIB_LINKAGE void*               HeapFree (HHEAP heap, void* _Ptr);

LIB_LINKAGE void*               HeapAllocGlobal (size_t _AllocationSize, u32 _AllocationAlignment = 16);
LIB_LINKAGE void*               HeapReallocGlobal (void *_Ptr, size_t _NewSize, u32 _NewAlignment = 16);
LIB_LINKAGE void*               HeapFreeGlobal (void* _Ptr);
}