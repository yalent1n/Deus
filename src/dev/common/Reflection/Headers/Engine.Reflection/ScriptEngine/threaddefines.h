#pragma once
#include "mmudefines.h"

/*
    Thread defines
*/

// NOTE: Defined by how many threads can be set on a single logical page.
#define SC_MAX_THREAD_COUNT             (_MMU_LOGICAL_PAGE_SIZE / sizeof( scrValue ))

// NOTE: Defined by how many locals can be set on a single effective page.
#define SC_MAX_THREAD_LOCAL_COUNT       (_MMU_EFFECTIVE_PAGE_SIZE / sizeof( scrValue ))