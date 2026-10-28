#pragma once
#include "../Bundle.h"
#include "registerdefines.h"

/*
    Script MMU properties
*/
#define _MMU_LOGICAL_OFFSET             14
#define _MMU_LOGICAL_WIDTH              14
#define _MMU_LOGICAL_LEVELS             2
#define _MMU_LOGICAL_LMASK              ((1 << _MMU_LOGICAL_WIDTH) - 1)
#define _MMU_LOGICAL_PAGE_SIZE          (1 << _MMU_LOGICAL_OFFSET)

/*
    Hardware MMU properties
*/
#define _MMU_EFFECTIVE_OFFSET           12
#define _MMU_EFFECTIVE_WIDTH            9
#define _MMU_EFFECTIVE_LEVELS           4
#define _MMU_EFFECTIVE_LMASK            ((1 << _MMU_EFFECTIVE_WIDTH) - 1)
#define _MMU_EFFECTIVE_PAGE_SIZE        (1 << _MMU_EFFECTIVE_OFFSET)

/*
    Translation Table defines
*/
#define TTx(a)  (((size_t)(a)) >> _MMU_LOGICAL_OFFSET)
#define TTE0(a) ((TTx(a) >> (0*_MMU_LOGICAL_WIDTH)) & _MMU_LOGICAL_LMASK)
#define TTE1(a) ((TTx(a) >> (1*_MMU_LOGICAL_WIDTH)) & _MMU_LOGICAL_LMASK)

/*
    Page Table defines
*/
#define PTx(a)  (((size_t)(a)) >> _MMU_EFFECTIVE_OFFSET)
#define PTE0(a) ((PTx(a) >> (0*_MMU_EFFECTIVE_WIDTH)) & _MMU_EFFECTIVE_LMASK)
#define PTE1(a) ((PTx(a) >> (1*_MMU_EFFECTIVE_WIDTH)) & _MMU_EFFECTIVE_LMASK)
#define PTE2(a) ((PTx(a) >> (2*_MMU_EFFECTIVE_WIDTH)) & _MMU_EFFECTIVE_LMASK)
#define PTE3(a) ((PTx(a) >> (3*_MMU_EFFECTIVE_WIDTH)) & _MMU_EFFECTIVE_LMASK)