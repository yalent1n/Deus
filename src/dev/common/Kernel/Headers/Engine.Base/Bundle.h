#pragma once
#include "Types.h"
#include "Platform.h"
#include "Log.h"
//#include "SmartRef.h"
#include "Endian.h"
#include "Misc/Sort.h"
#include "Misc/Vars.h"
#include "String/eastl_string.h"

#if defined(ENGINE_BASE_EXPORTS)
    #define FB_BASE_API LIB_LINKAGE
#elif defined(ENGINE_BUILD_STATIC)
    #define FB_BASE_API
#else
    #ifdef SvcCompilerMSVC
        #define FB_BASE_API __declspec(dllimport)
    #else
        #define FB_BASE_API __attribute__((visibility("default")))
    #endif
#endif

namespace rw
{
enum OwnershipPolicy : u8
{
    kNoOwnership        = 1 << 0,
    kAssumeOwnership    = 1 << 1,
    kSharesOwnership    = 1 << 2
};
}