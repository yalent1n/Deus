#include <Engine.Network/Bundle.h>
#include <Engine.Core/diag/ExecutionContext.h>

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#include <wolfssl/ssl.h>

FB_IMPLEMENT_MODULE(Network, "Networking package");

namespace rw
{
namespace winsock
{
    int pullup(int type, int proto, bool nonblock, bool async)
    {
        int ret;

        SOCKET fd = WSASocketA(AF_INET, type, proto, NULL, 0, (async ? WSA_FLAG_OVERLAPPED : 0));
        EASSERT(fd != INVALID_SOCKET, "WSASocketA failed");

        ret = SetHandleInformation((HANDLE)fd, HANDLE_FLAG_INHERIT, 0);
        EASSERT(ret, "SetHandleInformation failed");

        int nz = 0;
        ret = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char*)&nz, sizeof(nz));
        EASSERT(ret != SOCKET_ERROR, "setsockopt(SO_SNDBUF) failed");

        if(nonblock)
        {
            u_long bits = 1;
            ret = ioctlsocket(fd, FIONBIO, &bits);
            EASSERT(ret != SOCKET_ERROR, "ioctlsocket(FIONBIO) failed");
        }
        
        return int(fd);
    }
}
    
inline constexpr const char* CERT_PEM_PATH = "data:/Artifacts/.private/cert.pem";
inline constexpr const char* KEY_PEM_PATH = "data:/Artifacts/.private/key.pem";

const SharedBuffer* Net_GetCertPem()
{
return ExecutionContext::getVirtualFileSystem()->shareFile(CERT_PEM_PATH);
}

const SharedBuffer* Net_GetKeyPem()
{
    return ExecutionContext::getVirtualFileSystem()->shareFile(KEY_PEM_PATH);
}

void* CSSkywalkStart()
{
    static WSAData wd {};
    (void) WSAStartup(MAKEWORD(2, 2), &wd);

    if(wolfSSL_Init() != WOLFSSL_SUCCESS)
        return nullptr;

    return wolfSSL_CTX_new(wolfTLSv1_3_client_method());
}

bool CSSkywalkStop()
{
    return wolfSSL_Cleanup() == WOLFSSL_SUCCESS;
}
}