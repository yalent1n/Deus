#include <Engine.Core/Bundle.h>
#include <Engine.Base/String/String.h>

#if defined(ENGINE_SATURN_EXPORTS)
    #define SATURN_API LIB_LINKAGE
#elif defined(ENGINE_BUILD_STATIC)
    #define SATURN_API
#else
    #ifdef SvcCompilerMSVC
        #define SATURN_API __declspec(dllimport)
    #else
        #define SATURN_API __attribute__((visibility("default")))
    #endif
#endif

#define FB_CoreSaturn_API SATURN_API
FB_DECLARE_MODULE(CoreSaturn)
#undef FB_CoreSaturn_API

using namespace rw;