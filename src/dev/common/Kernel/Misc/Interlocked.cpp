#include <Engine.Base/Macros.h>

#if defined(SvcCompilerMSVC)
    #include "Interlocked_msvc.cpp"
#else
    #error "Unimplemented platform for Interlocked Ops from Esis"
#endif