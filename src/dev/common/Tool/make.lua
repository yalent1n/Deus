dofile 'X:/dev/elsa/tools/Scripts/Build/setenv.lua'

init {
    id = 'common/Tool',
    suffix = 'framework'
}

configure {

    defines = {
        'ENGINE_BUILD_DLL',
        'ENGINE_TOOLCORE_EXPORTS'
    },

    framework_dependencies = {
        'common/Kernel',
        'common/System'
    },

    system_dependencies = {
        'Advapi32'
    },

    files = {
        'Bundle.cpp',
        'CompressionHelper.cpp',

        'App/ServiceBase.cpp',
        'App/ApplicationBase.cpp'
    }
}

compile_apppackage {
    cc = 'clang++',
    std = 'c++17',
    warnings = { 'all' }
}