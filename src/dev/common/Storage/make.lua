dofile 'X:/dev/elsa/tools/Scripts/Build/setenv.lua'

init {
    id = 'common/Storage',
    suffix = 'framework'
}

configure {
    defines = {
        'ENGINE_BUILD_DLL',
        'ENGINE_STORAGE_EXPORTS',
        'ENGINE_DEBUG_BUILD'
    },

    framework_dependencies = {
        'common/Kernel',
        'common/System',
        'common/Tool',
        'common/Network'
    },

    system_dependencies = {
        'Advapi32',
        'bcrypt'
    },

    files = {
        'Bundle.cpp',

        'Delta.cpp',
        'Bulk.cpp',

        'Deployment/BuildLayer.cpp',
        'Deployment/BuildImage.cpp',
        'Deployment/Deploy.cpp',

        'StoragePool.cpp',
        'Pools/FileStoragePool.cpp',
        'Pools/FlexibleStoragePool.cpp',
        'Pools/RemoteStoragePool.cpp',
        'LocalChunkStore.cpp',
        
        -- 'IoTrustVault.cpp',
        -- 'IoService.cpp',
        -- 'IoStream.cpp'
    }
}

compile_apppackage {
    cc = 'clang++',
    std = 'c++17',
    warnings = { 'all' }
}