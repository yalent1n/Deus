dofile 'X:/dev/elsa/tools/Scripts/Build/setenv.lua'

init {
    id = 'common/ExchangeKit',
    suffix = 'framework'
}

configure {

    defines = {
        'ENGINE_BUILD_DLL',
        'ENGINE_DEBUG_BUILD',
        'ENGINE_SATURN_EXPORTS',
        '_WINSOCK_DEPRECATED_NO_WARNINGS',
        'WOLFSSL_TLS13',
        'WC_NO_HARDEN'
    },

    framework_dependencies = {
        'common/Kernel',
        'common/System',
        'common/Network'
    },

    system_directories = {
        BUILD_ROOT .. 'External'
    },

    system_dependencies = {
        'Advapi32'
    },

    files = {
        'Module.cpp',
        'DirectoryRecord.cpp',
        'ObjectProvider.cpp',
        'DataProvider.cpp',
        'Management.cpp'
    }
}

compile_apppackage {
    cc = 'clang++',
    std = 'c++17',
    warnings = { 'all' }
}