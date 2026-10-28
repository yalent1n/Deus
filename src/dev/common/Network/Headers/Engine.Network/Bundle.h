#pragma once
#include <Engine.Core/Bundle.h>
#include <Engine.Base/Buffer/SharedBuffer.h>
#include <Engine.Base/Ref.h>
#include <Engine.Base/HashMap.h>

#ifdef SvcCompilerMSVC
    #define WIN32_LEAN_AND_MEAN
    #include <WinSock2.h>
    #include <Windows.h>
#endif

#if defined(ENGINE_NETWORK_EXPORTS)
    #define FB_NETWORK_API LIB_LINKAGE
#elif defined(ENGINE_BUILD_STATIC)
    #define FB_NETWORK_API
#else
    #ifdef SvcCompilerMSVC
        #define FB_NETWORK_API __declspec(dllimport)
    #else
        #define FB_NETWORK_API __attribute__((visibility("default")))
    #endif
#endif

#define FB_Network_API FB_NETWORK_API
FB_DECLARE_MODULE(Network)
#undef FB_Network_API

namespace rw
{
namespace winsock
{
	FB_NETWORK_API int pullup(int type = SOCK_STREAM, int proto = IPPROTO_TCP, bool nonblock = true, bool async = false);
}

FB_NETWORK_API const SharedBuffer* Net_GetCertPem();
FB_NETWORK_API const SharedBuffer* Net_GetKeyPem();

FB_NETWORK_API void* CSSkywalkStart();
FB_NETWORK_API bool CSSkywalkStop();
}