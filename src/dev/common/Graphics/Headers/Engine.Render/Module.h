#pragma once
#include <Engine.Core/Bundle.h>
#include <Engine.Base/String/String.h>
#include <Engine.Base/String/CString.h>
#include <Engine.Base/HashMap.h>
#include <Engine.Base/Misc/FastDelegate.h>

#if defined(ENGINE_RENDER_EXPORTS)
    #define FB_RENDER_API LIB_LINKAGE
#elif defined(ENGINE_BUILD_STATIC)
    #define FB_RENDER_API
#else
    #ifdef SvcCompilerMSVC
        #define FB_RENDER_API __declspec(dllimport)
    #else
        #define FB_RENDER_API __attribute__((visibility("default")))
    #endif
#endif

#define FB_Render_API FB_RENDER_API
FB_DECLARE_MODULE(Render)
#undef FB_Render_API

namespace rw
{   
FB_RENDER_API void coreRenderInit(int w, int h, const char* title);
FB_RENDER_API void coreRenderShutdown();
FB_RENDER_API bool coreRenderShouldClose();

FB_RENDER_API void coreRenderGetFramebufferSize(int* w, int* h);
FB_RENDER_API void coreRenderSwapBuffers();
}