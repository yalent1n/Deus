@echo off
set project=Engine
set module=Render
set ext=dll

SET code=%CODE_ROOT%/%project%/%module%
SET build=%BUILD_ROOT%/%project%

SET CC=clang++
set DEFS=-DENGINE_BUILD_DLL -DENGINE_DEBUG_BUILD -DENGINE_RENDER_EXPORTS
set CCFLAGS=-std=c++17 -Wall -shared -g %DEFS%
set INCLUDES=-I%CODE_ROOT%/Engine/Base/include -I%CODE_ROOT%/Engine/Core/include -I%CODE_ROOT%/Engine/ToolCore/include -I%CODE_ROOT%/Engine/Network/include -I%code%/include -I%code%/include/imgui
set SEARCH_PATHS=-L%build% -L%code%/glfw/lib-vc2022
set DEPS=-lEngine.Base -lEngine.Core -lEngine.ToolCore -lEngine.Network -lAdvapi32  -lopengl32 -lgdi32 -lglfw3_mt -lbcrypt

set TARGET=%build%/%project%.%module%.%ext%

set imgui=%code%/imgui
set ImGui_s=%imgui%/imgui.cpp %imgui%/imgui_demo.cpp %imgui%/imgui_draw.cpp %imgui%/imgui_tables.cpp %imgui%/imgui_widgets.cpp %imgui%/backends/imgui_impl_glfw.cpp %imgui%/backends/imgui_impl_opengl3.cpp

set Events_s=%code%/EventQueue.cpp
set RenderSurface_s=%code%/WidgetDeclarations.cpp %code%/WidgetRenderSurface.cpp
set RenderDevice_s=%code%/WidgetRenderDevice.cpp

set SOURCES=%ImGui_s% %code%/Module.cpp %RenderSurface_s% %RenderDevice_s%

%CC% %CCFLAGS% %SEARCH_PATHS% %DEPS% %INCLUDES% -o %TARGET% %SOURCES%

IF %ERRORLEVEL% EQU 0 (
    cls
    @echo %project%.%module% has been built. Target: %TARGET%
) ELSE (
    @echo %project%.%module% has failed to build.
    exit /b 255
)