dofile 'X:/dev/elsa/tools/Scripts/Build/setenv.lua'

init {
    id = 'common/Kernel',
    suffix = 'framework'
}

configure {

    defines = {
        'ENGINE_BUILD_DLL',
        'ENGINE_BASE_EXPORTS',
        'ENGINE_DEBUG_BUILD'
    },

    includes = {
        SDK_ROOT .. 'CoreLibs/Headers'
    },

    system_directories = {
        SDK_ROOT .. 'CoreLibs/Binaries'
    },

    system_dependencies = {
        'Advapi32',
        'Bcrypt',
        'lz4',
        'zlib',
        'zstd'
    },

    files = {
        'Config.cpp',
        'Log.cpp',
        'Mime.cpp',
        'RegistryKey.cpp',

        'Buffer/SharedBuffer.cpp',
        'Buffer/Buffer.cpp',
        'Buffer/Adapter.cpp',
        'Buffer/RegistryBuffer.cpp',
        -- 'Buffer/Stream/EndianInStream.cpp',

        'Database/DbData.cpp ',
        'Database/DbOid.cpp ',
        'Database/DbObject.cpp ',
        'Database/DbObjectBuilder.cpp',

        'Memory/Util.cpp',
        'Memory/Map.cpp',
        'Memory/VirtualAlloc.cpp',
        'Memory/Allocator.cpp',
        'Memory/LinearAllocator.cpp',
        'Memory/PageAllocator.cpp',
        'Memory/HeapAllocator.cpp',

        'Thread/CriticalSection.cpp',
        'Thread/Thread.cpp',
        'Thread/WorkerThreadPool.cpp',

        'String/Util.cpp',
        'String/StringBuilder.cpp',
        'String/eastl_string.cpp',

        'Crypto/SHAFunctions.cpp',
        'Crypto/SHA.cpp',
        'Crypto/Base64.cpp',
        'Crypto/Guid.cpp',
        'Crypto/AES.cpp',
        'Crypto/HKDF.cpp',
        'Crypto/ECDH.cpp',
        'Crypto/ECC.cpp',

        'Misc/Time.cpp',
        'Misc/Vars.cpp',
        'Misc/Util.cpp',
        'Misc/Event.cpp',
        'Misc/Json.cpp',
        'Misc/Xml.cpp',
        'Misc/Ini.cpp',
        'Misc/gzip.cpp',
        'Misc/Codec.cpp',
        'Misc/Interlocked.cpp'
    }
}

compile_apppackage {
    cc = 'clang++',
    std = 'c++17',
    warnings = { 'all' }
}