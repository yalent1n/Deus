#pragma once
#include <Engine.Core/Bundle.h>
#include <Engine.Base/Vec.h>
#include <Engine.Base/FixedList.h>
#include <Engine.Base/HashMap.h>

#if defined(ENGINE_REFLECTION_EXPORTS)
    #define FB_REFL_API LIB_LINKAGE
#elif defined(ENGINE_BUILD_STATIC)
    #define FB_REFL_API
#else
    #ifdef SvcCompilerMSVC
        #define FB_REFL_API __declspec(dllimport)
    #else
        #define FB_REFL_API __attribute__((visibility("default")))
    #endif
#endif

#define FB_Reflection_API LIB_LINKAGE
FB_DECLARE_MODULE(Reflection)
#undef FB_Reflection_API

namespace rw
{
template <bool C=true> class SymbolHandle;
template <class T, bool C=true> class SmartHandle;

template <typename T> class Array;
template <typename T> class ArrayReader;
template <typename T> class ArrayEditor;
}