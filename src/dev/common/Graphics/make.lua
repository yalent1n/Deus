dofile 'X:/dev/elsa/tools/Scripts/Build/setenv.lua'

init {
    id = 'common/Graphics',
    suffix = 'framework'
}

configure {
    includes = {
        AppPackage.Bundle.CodeRoot .. 'Headers/imgui'
    },

    defines = {
        'ENGINE_BUILD_DLL',
        'ENGINE_RENDER_EXPORTS',
        'ENGINE_DEBUG_BUILD'
    },

    framework_dependencies = {
        'common/Kernel',
        'common/System',
        'common/Tool'
    },

    system_directories = {
        AppPackage.Bundle.CodeRoot .. 'glfw/lib-vc2022'
    },

    system_dependencies = {
        'Advapi32',
        'opengl32',
        'gdi32',
        'glfw3_mt'
    },

    files = {
        'Module.cpp',
        
        'imgui/imgui.cpp',
        'imgui/imgui_demo.cpp',
        'imgui/imgui_draw.cpp',
        'imgui/imgui_tables.cpp',
        'imgui/imgui_widgets.cpp',
        'imgui/backends/imgui_impl_glfw.cpp',
        'imgui/backends/imgui_impl_opengl3.cpp',

        'EventQueue.cpp',
        'WidgetDeclarations.cpp',
        'WidgetRenderSurface.cpp',
        'WidgetRenderDevice.cpp'
    }
}

compile_apppackage {
    cc = 'clang++',
    std = 'c++17',
    warnings = { 'all' }
}