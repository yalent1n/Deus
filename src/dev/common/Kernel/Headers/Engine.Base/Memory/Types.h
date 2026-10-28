#pragma once
#include "../Bundle.h"

#define vm_sys_err(l) (-l)
#define vm_page_err(n) (-100 -n)

// System errors
#define VMESNOL4            vm_sys_err(0)
#define VMESNODPT           vm_sys_err(1)
#define VMESNODT            vm_sys_err(2)

// Page access errors
#define VMEPNOPRT           vm_page_err(0)
#define VMEPNOEND           vm_page_err(1)
#define VMEPNOWR            vm_page_err(2)
#define VMEPNOEXEC          vm_page_err(3)
#define VMEPNOEFN           vm_page_err(4)

#define VMENOFAULT          1

namespace rw
{
inline constexpr size_t kPageSize = 0x1000;
inline constexpr size_t kMaxPageTableEntry = kPageSize / 8;

typedef struct _mm_pte
{
    u64 present : 1;
    u64 writable : 1;
    u64 useraccess : 1;
    u64 region : 1;
    u64 reserved_0 : 1; // nocache
    u64 accessed : 1;
    u64 dirty : 1;
    u64 efn : 36; // End Frame Number (in Region mode)
    u64 noexecution : 1;
    u64 lfn : 20; // Logical Frame number (for support in scripting)
} mm_pte, PTE, *PPTE;

typedef struct _mm_page_information mm_page_information, PAGE_INFORMATION, *PPAGE_INFORMATION;

union mm_page_item
{
    struct _mm_pte i_pte;
    struct _mm_page_information* i_indir;
};

struct _mm_page_information
{    
    mm_page_item i_pages[kMaxPageTableEntry];
};
}