dofile 'X:/dev/elsa/tools/Scripts/Build/setenv.lua'

init {
    id = 'common/Network',
    suffix = 'framework'
}

configure {

    defines = {
        'ENGINE_BUILD_DLL',
        'ENGINE_DEBUG_BUILD',
        'ENGINE_NETWORK_EXPORTS',
        '_WINSOCK_DEPRECATED_NO_WARNINGS',
        'WOLFSSL_TLS13',
        'WC_NO_HARDEN'
    },

    framework_dependencies = {
        'common/Kernel',
        'common/System'
    },

    system_directories = {
        SDK_ROOT .. 'CoreLibs/Binaries'
    },

    system_dependencies = {
        'Advapi32',
        'wolfssl',
        "Bcrypt"
    },

    files = {
        'Bundle.cpp',
        'HttpSession.cpp',
        'SocketBase.cpp',
        'Util.cpp',
        'Netcrypt.cpp',

        'TcpClientConnection.cpp',
        'TcpServer.cpp',
        'TcpServerConnection.cpp',
        'TcpServerConnectionImpl.cpp',

        'Relay.cpp',

        'http/base.cpp',
        'http/router.cpp',
        'http/server.cpp',

        'aidl/frame.cpp',
        'aidl/schedule.cpp',
        'aidl/sync.cpp',
        'aidl/atlas.cpp',
        'aidl/expansion.cpp'
    }
}

compile_apppackage {
    cc = 'clang++',
    std = 'c++17',
    warnings = { 'all' }
}