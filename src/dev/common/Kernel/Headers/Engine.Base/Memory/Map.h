#pragma once
#include "Types.h"

namespace rw
{
// Memory management control Request
struct mmctl_req
{
    u8 writable : 1;
    u8 executable : 1;
    u8 user : 1;
};

class FB_BASE_API atlMemManager
{
public:
    atlMemManager();

// Mapping functions
public:
    PPTE        MapPage(const void* va, mmctl_req req);
    void        UnMapPage(const void* va);
    
    PPTE        MapPages(const void* va_start, const void* va_end, mmctl_req req);
    int         UnMapPages(const void* va_start);

// Static functions
public:
    static void*        SystemAlloc(size_t size, bool commit = true);
    static void         SystemFree(void* va);

// Allocation functions
public:
    void* Allocate(size_t size, bool exec = false);
    void free(void* va);

    _mm_page_information mGDT; // Global Descriptor Table
};

FB_BASE_API extern atlMemManager sysMem;

FB_BASE_API void*   MmAllocate(size_t);
FB_BASE_API void    MmUnAllocate(void*);
}