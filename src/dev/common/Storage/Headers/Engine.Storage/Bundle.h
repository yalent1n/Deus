#pragma once
#include <Engine.Core/Bundle.h>
#include <Engine.Base/Ref.h>
#include <Engine.Base/Crypto/SHA.h>

#if defined(ENGINE_STORAGE_EXPORTS)
    #define FB_STORAGE_API LIB_LINKAGE
#elif defined(ENGINE_BUILD_STATIC)
    #define FB_STORAGE_API
#else
    #ifdef SvcCompilerMSVC
        #define FB_STORAGE_API __declspec(dllimport)
    #else
        #define FB_STORAGE_API __attribute__((visibility("default")))
    #endif
#endif

#define FB_Storage_API FB_STORAGE_API
FB_DECLARE_MODULE(Storage)
#undef FB_Storage_API