#include <Engine.Base/Memory/Map.h>
#include <stdio.h>

#ifdef SvcCompilerMSVC
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

#define S_PTE 12
#define B_PTE 9
#define _MMU_EFFECTIVE_LMASK ((1 << B_PTE) - 1)

#define PTx(a)  (((size_t)(a)) >> S_PTE)
#define PTE0(a) ((PTx(a) >> (0*B_PTE)) & _MMU_EFFECTIVE_LMASK)
#define PTE1(a) ((PTx(a) >> (1*B_PTE)) & _MMU_EFFECTIVE_LMASK)
#define PTE2(a) ((PTx(a) >> (2*B_PTE)) & _MMU_EFFECTIVE_LMASK)
#define PTE3(a) ((PTx(a) >> (3*B_PTE)) & _MMU_EFFECTIVE_LMASK)

namespace rw
{
atlMemManager sysMem = {};

PPAGE_INFORMATION VmcRequest()
{
    mm_page_information* pagetable = (mm_page_information*) atlMemManager::SystemAlloc(kPageSize);
    for(size_t i = 0; i < kMaxPageTableEntry; i++)
        pagetable->i_pages[i].i_indir = NULL;

    return pagetable;
}

static __forceinline int VmcLookup(struct _mm_page_information* Gdt, const void* va, PPTE* out, bool checkPresent, bool checkRegion = false)
{
    s16 PTE = PTE3(va),
        PDTE = PTE2(va),
        PDPTE = PTE1(va),
        PML4E = PTE0(va);

    PPAGE_INFORMATION Pdt = NULL, Pdpt = NULL, Pml4 = NULL;

    Pml4 = Gdt->i_pages[PML4E].i_indir;
    if(!Pml4) return VMESNOL4;        
    Pdpt = Pml4->i_pages[PDPTE].i_indir;
    if(!Pdpt) return VMESNODPT;
    Pdt = Pdpt->i_pages[PDTE].i_indir;
    if(!Pdt) return VMESNODPT;
    PPTE pte = &Pdt->i_pages[PTE].i_pte;
    if(!pte->present) return VMEPNOPRT;
    if(!pte->region) return VMEPNOEND;
    *out = pte;
    if(checkPresent && !pte->present) return VMEPNOPRT;
    if(checkRegion && !pte->region) return VMEPNOEND;

    return VMENOFAULT;
}

void* atlMemManager::SystemAlloc(size_t size, bool commit)
{
#ifdef SvcCompilerMSVC
    return VirtualAlloc(NULL, size, MEM_RESERVE | __maybe(commit, MEM_COMMIT), PAGE_READWRITE);
#else
    // TODO: unix.
    return nullptr;
#endif
}

void atlMemManager::SystemFree(void* va)
{
#ifdef SvcCompilerMSVC
    VirtualFree(va, 0, MEM_RELEASE);
#else
    // TODO: unix.
    return;
#endif
}

atlMemManager::atlMemManager()
    : mGDT {}
{}

PPTE atlMemManager::MapPage(const void* va, mmctl_req req)
{
    s16 PTE = PTE3(va),
        PDTE = PTE2(va),
        PDPTE = PTE1(va),
        PML4E = PTE0(va);

    PPAGE_INFORMATION* Pdt = NULL, *Pdpt = NULL, *Pml4 = NULL;

    Pml4 = &mGDT.i_pages[PML4E].i_indir;
    if(!*Pml4) {
        *Pml4 = VmcRequest();
    }
    Pdpt = &(*Pml4)->i_pages[PDPTE].i_indir;
    if(!*Pdpt) {
        *Pdpt = VmcRequest();
    }    
    Pdt = &(*Pdpt)->i_pages[PDTE].i_indir;
    if(!*Pdt) {
        *Pdt = VmcRequest();
    }    
    
    PPTE Pte = &(*Pdt)->i_pages[PTE].i_pte;
    Pte->present = 1;
    Pte->writable = req.writable;
    Pte->useraccess = req.user;
    Pte->noexecution = !req.executable ? 1 : 0;
    return Pte;
}

void atlMemManager::UnMapPage(const void* va)
{
    // i16 PTE = PTE3(va),
    //     PDTE = PTE2(va),
    //     PDPTE = PTE1(va),
    //     PML4E = PTE0(va);

    struct _mm_pte* pte = nullptr;
    if(int err = VmcLookup(&mGDT, va, &pte, true, false); err <= 0)
    {
        return;
    }

    pte->present = 0;    
}

PPTE atlMemManager::MapPages(const void* va_start, const void* va_end, mmctl_req req)
{
    u64 efn = (((u64)va_end + kPageSize - 1) & ~(kPageSize - 1)) >> S_PTE;

    u8* va = (u8*)va_start;
    PPTE pte = MapPage(va, req);
    pte->region = 1;
    pte->efn = efn;
    va += kPageSize;

    while(va < va_end) {
        PPTE pte = MapPage(va, req);
        pte->region = 1;
        va += kPageSize;
    }

    return pte;
}

int atlMemManager::UnMapPages(const void* va_start)
{
    u8* va = (u8*)va_start;

    struct _mm_pte* pte = nullptr;
    if(int err = VmcLookup(&mGDT, va, &pte, true, true); err <= 0) {
        return err;
    }

    if(!pte->efn) {
        return VMEPNOEFN;
    }

    u8* va_end = (u8*)((u64)pte->efn << S_PTE);
    pte->efn = 0;
    //printf("Region start : %p\nRegion end :   %p\n", va_start, va_end);

    while(va < va_end)
    {
        if(int err = VmcLookup(&mGDT, va, &pte, true, true); err <= 0) {
            return err;
        }
                
        pte->present = 0;
        pte->region = 0;
        va += kPageSize;
    }

    return VMENOFAULT;
}

void* atlMemManager::Allocate(size_t size, bool exec)
{
    mmctl_req req;
    req.writable = 1;
    req.executable = exec;
    req.user = 1;

    void* va_start = SystemAlloc(size);
    if(!va_start) {
        return nullptr;
    }
    if(size < kPageSize)
    {
        MapPage(va_start, req);
    }
    else
    {
        void* va_end = (u8*)va_start + size;
        MapPages(va_start, va_end, req);
    }

    return va_start;
}

void atlMemManager::free(void* va)
{
    int ret = UnMapPages(va);
    if(ret <= 0) {
        //printf("Failed to free memory region : %d\n", ret);
        return;
    }

    SystemFree(va);
}

void* MmAllocate(size_t size)
{
    return sysMem.Allocate(size);
}

void MmUnAllocate(void* addr)
{
    sysMem.free(addr);
}
}