#pragma once
#include <Engine.Core/Bundle.h>

#if defined(ENGINE_TOOLCORE_EXPORTS)
    #define CORETOOL_API LIB_LINKAGE
#elif defined(ENGINE_BUILD_STATIC)
    #define CORETOOL_API
#else
    #ifdef SvcCompilerMSVC
        #define CORETOOL_API __declspec(dllimport)
    #else
        #define CORETOOL_API __attribute__((visibility("default")))
    #endif
#endif

#define FB_ToolCore_API CORETOOL_API
FB_DECLARE_MODULE(ToolCore)
#undef FB_ToolCore_API