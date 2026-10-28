#include <Engine.Base/Memory/VirtualAlloc.h>
#include <Engine.Base/Memory/Map.h>
#include <Engine.Base/Memory/Allocator.h>
#include <stdio.h>

#ifdef SvcCompilerMSVC
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

namespace rw
{
typedef struct _IOMEMORYBLOCK {
    u8* PointerToBlock;
    size_t SizeOfBlock;
    struct _IOMEMORYBLOCK* PointerToNextBlock;
} IOMemoryBlock, *PIOMemoryBlock;

typedef struct _IOMEMORYCONTROLLER {
    u8 *heapBegin;
    u8 *heapCursor;
    IOMemoryBlock *blocksHead;
} IOMemoryController, *PIOMemoryController;

//size_t _x = sizeof(_IOMEMORYCONTROLLER);

static inline HHEAP GlobalHeap = nullptr;
static inline IOMemoryController GLOBAL;

DWORD CreateHeapObject(HHEAP _Heap, size_t _VirtualSpace)
{
    IOMemoryController* iomem = (IOMemoryController*) _Heap;
    iomem->heapBegin = (u8*) sysMem.SystemAlloc(_VirtualSpace);
    if(!iomem->heapBegin)
    {
        return false;
    }

    iomem->heapCursor = iomem->heapBegin;

    mmctl_req req;
    req.executable = 0;
    req.user = 1;
    req.writable = 1;
    sysMem.MapPages(iomem->heapBegin, iomem->heapBegin + _VirtualSpace, req);
    return true;
}

DWORD DestroyHeapObject(HHEAP _Heap)
{
    _IOMEMORYCONTROLLER* IOMC = (IOMemoryController*) _Heap;

    if(IOMC->heapBegin)
    {
        _IOMEMORYBLOCK* IOMB = IOMC->blocksHead;
        while(IOMB)
        {
            _IOMEMORYBLOCK* Next = IOMB->PointerToNextBlock;
            delete IOMB;
            IOMB = Next;
        }

        if(sysMem.UnMapPages(IOMC->heapBegin) < 0)
        {
            return false;
        }

        sysMem.SystemFree(IOMC->heapBegin);
        IOMC->heapBegin = IOMC->heapCursor = NIL;
    }

    return true;
}

HHEAP ObtainGlobalHeapHandle()
{
    return GlobalHeap;
}

int InitGlobalHeap()
{
    GlobalHeap = &GLOBAL;
    return CreateHeapObject(GlobalHeap, kVirtualSpaceMax);
}

int ShutdownGlobalHeap()
{
    DWORD dwResult = DestroyHeapObject(GlobalHeap);
    GlobalHeap = NIL;
    return dwResult;
}

#define alias(v, t) ((t)(v))

void* HeapAlloc(HHEAP _Heap, size_t _AllocationSize, u32 _AllocationAlignment)
{
    if(_AllocationAlignment == NIL) {
        _AllocationAlignment = 1;
    }

    _IOMEMORYCONTROLLER* IOMC = alias(_Heap, _IOMEMORYCONTROLLER*);

    u8* alignedPtr = (u8*)((word)IOMC->heapCursor + (_AllocationAlignment - 1) & (~((word)_AllocationAlignment - 1)));
    IOMC->heapCursor = alignedPtr + _AllocationSize;

    PIOMemoryBlock Hdr = new IOMemoryBlock();
    Hdr->PointerToBlock = alignedPtr;
    Hdr->SizeOfBlock = _AllocationSize;
    Hdr->PointerToNextBlock = IOMC->blocksHead;
    IOMC->blocksHead = Hdr;
    return alignedPtr;
}

size_t HeapQuerySize(HHEAP _Heap, void* _Ptr)
{
    _IOMEMORYCONTROLLER* IOMC = alias(_Heap, _IOMEMORYCONTROLLER*);

    if(IOMC->blocksHead)
    {
        for(PIOMemoryBlock Hdr = IOMC->blocksHead; Hdr; Hdr = Hdr->PointerToNextBlock)
        {
            if(Hdr->PointerToBlock == _Ptr)
                return Hdr->SizeOfBlock;
        }
    }
    
    return 0;
}

void* HeapRealloc(HHEAP _Heap, void* _OldPtr, size_t _NewSize, u32 _Alignment)
{
    _IOMEMORYCONTROLLER* IOMC = alias(_Heap, _IOMEMORYCONTROLLER*);

    if(!IOMC->blocksHead)
    {
        return NIL;
    }

    PIOMemoryBlock _Old = NIL;
    for(PIOMemoryBlock Hdr = IOMC->blocksHead; Hdr; Hdr = Hdr->PointerToNextBlock)
    {
        if(Hdr->PointerToBlock == _OldPtr)
        {
            _Old = Hdr;
            break;
        }
    }

    if(!_Old)
    {
        return NIL;
    }

    if(_Alignment == NIL) {
        _Alignment = 1;
    }

    u8* _NewPtr = (u8*)((word)IOMC->heapCursor + (_Alignment - 1) & (~((word)_Alignment - 1)));
    IOMC->heapCursor = _NewPtr + _NewSize;
    memcpy(_NewPtr, _Old->PointerToBlock, _Old->SizeOfBlock);

    _Old->PointerToBlock = _NewPtr;
    _Old->SizeOfBlock = _NewSize;
    return _NewPtr;
}

void* HeapFree(HHEAP _Heap, void* _Ptr)
{
    _IOMEMORYCONTROLLER* IOMC = alias(_Heap, _IOMEMORYCONTROLLER*);

    // Pointers to traverse the list
    IOMemoryBlock* Prev = NIL;
    IOMemoryBlock* Current = IOMC->blocksHead;
    
    // Case 1: Removing the head node
    if (Current && Current->PointerToBlock == _Ptr) {
        IOMC->blocksHead = Current->PointerToNextBlock; // Change head
        delete Current; // Free old head
        return _Ptr;
    }
    
    // Case 2: Search for the node to be removed
    while (Current && Current->PointerToBlock != _Ptr) {
        Prev = Current;
        Current = Current->PointerToNextBlock;
    }
    
    // If the value was not found
    if (Current == NIL) {
        return NIL;
    }
    
    // Unlink the node from the list
    Prev->PointerToNextBlock = Current->PointerToNextBlock;
    
    // better to zero out unused area
    std::memset(Current->PointerToBlock, 0, Current->SizeOfBlock);

    void* Ret = Current->PointerToBlock;
    delete Current;
    return Ret;
}

#ifdef ENGINE_HAS_GLOBAL_HEAP

void* HeapAllocGlobal(size_t _AllocationSize, u32 _AllocationAlignment)
    { __builtin_assert(GlobalHeap); return HeapAlloc(GlobalHeap, _AllocationSize, _AllocationAlignment); }

void* HeapReallocGlobal(void* _Ptr, size_t _NewSize, u32 _NewAlignment)
{ 
    __builtin_assert(GlobalHeap);

    size_t _OldSize = HeapQuerySize(GlobalHeap, _Ptr);
    if(_OldSize == 0)
        return nullptr;

    void* _NewPtr = HeapAlloc(GlobalHeap, _NewSize, _NewAlignment);
    std::memcpy(_NewPtr, _Ptr, _OldSize);
    return _NewPtr;
}

void* HeapFreeGlobal(void* _Ptr)
    { __builtin_assert(GlobalHeap); return HeapFree(GlobalHeap, _Ptr); }

#else

void* HeapAllocGlobal(size_t _AllocationSize, u32 _AllocationAlignment)
    { return sysAllocator::GetCurrent()->Allocate(_AllocationSize, _AllocationAlignment, MEMTYPE_VIRTUAL); }

void* HeapReallocGlobal(void* _Ptr, size_t _NewSize, u32 _NewAlignment)
{
    size_t _OldSize = sysAllocator::GetCurrent()->QueryAllocationSize(_Ptr, MEMTYPE_VIRTUAL);
    if(_OldSize == 0)
        return nullptr;

    void* _NewPtr = sysAllocator::GetCurrent()->Allocate(_NewSize, _NewAlignment, MEMTYPE_VIRTUAL);
    std::memcpy(_NewPtr, _Ptr, _OldSize);
    return _NewPtr;
}

void* HeapFreeGlobal(void* _Ptr)
    { return sysAllocator::GetCurrent()->Free(_Ptr, MEMTYPE_VIRTUAL) ? _Ptr : NULL; }

#endif
}