dofile 'X:/dev/elsa/tools/Scripts/Build/setenv.lua'

init {
    id = 'apps/CanopyServer',
    suffix = 'app'
}

configure {
    defines = {
        'WOLFSSL_TLS13',
        '_CRT_SECURE_NO_WARNINGS',
        'SvcAppPROJECT=' .. AppPackage.Bundle.id,
        'SvcAppMODULE=' .. AppPackage.Bundle.name,
        'SvcAppBRANCH=' .. LOCAL_USER_BRANCH
    },

    framework_dependencies = {
        'common/Kernel',
        'common/System',
        'common/Tool',
        'common/Network',
        'common/Graphics',
        'common/Reflection',
        'common/Storage',
        'common/ExchangeKit'
    },

    system_dependencies = {
        'Advapi32',
        'User32',
        'Bcrypt'
    },

    files = {
        'App.cpp',
        'main.cpp',
        'Startup.cpp',
        'DiskManagementService.cpp'
    }
}

compile_apppackage {
    cc = 'clang++',
    std = 'c++17',
    warnings = { 'all' }
}