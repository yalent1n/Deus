dofile 'X:/dev/elsa/tools/Scripts/Build/setenv.lua'

init {
    id = 'common/System',
    suffix = 'framework'
}

configure {

    defines = {
        'ENGINE_BUILD_DLL',
        'ENGINE_CORE_EXPORTS',
        'ENGINE_DEBUG_BUILD',
        '_CRT_SECURE_NO_WARNINGS'
    },
    
    framework_dependencies = {
        'common/Kernel'
    },

    system_dependencies = {
        'Advapi32'
    },

    files = {
        'ModuleInfo.cpp',
        'Bundle.cpp',

        'VirtualFileSystem.cpp',
        'FsBackend.cpp',

        'diag/ExecutionContext.cpp',
        'diag/Environment.cpp',

        'Buffer/Win32/Win32Buffer.cpp',

        'Zip/ZipIo.cpp',
        'Zip/ZipFormat.cpp',

        'Backends/RegistryIoBackend.cpp',
        'Backends/Win32IoBackend.cpp'
    }

}

compile_apppackage {
    cc = 'clang++',
    std = 'c++17',
    warnings = { 'all' }
}