dofile 'X:/dev/elsa/tools/Scripts/Build/setenv.lua'

init {
    id = 'common/Reflection',
    suffix = 'framework'
}

configure {
    defines = {
        'ENGINE_BUILD_DLL',
        'ENGINE_REFLECTION_EXPORTS',
        'ENGINE_DEBUG_BUILD',
        '_CRT_SECURE_NO_WARNINGS'
    },

    framework_dependencies = {
        'common/Kernel',
        'common/System'
    },

    system_dependencies = {
        'Advapi32'
    },

    files = {
        'Compartment.cpp',

        'TypeInfo.cpp',
        'ArrayTypeInfo.cpp',
        'PrimitiveTypeInfo.cpp',
        'ValueTypeInfo.cpp',
        'ClassInfo.cpp',
        
        'NativeTypeRegistry.cpp',
        
        'ArrayBase.cpp',
        
        'SymbolClassInfo.cpp',
        'Symbol.cpp',
        'SmartHandle.cpp',

        'Asset.cpp',

        'ScriptEngine/mm.cpp',
        'ScriptEngine/image.cpp',
        
        'RuntimeIpr.cpp',
        'RuntimeDatabasePartition.cpp',
        'GenericDataLayout.cpp'
    }
}

compile_apppackage {
    cc = 'clang++',
    std = 'c++17',
    warnings = { 'all' }
}