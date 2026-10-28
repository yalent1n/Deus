@ECHO OFF
SET project=Finance
SET module=Accounting
SET branch=%LOCAL_USER_CONFIG%
SET ext=exe

SET ENGINE=%CODE_ROOT%/Engine

SET code=%CODE_ROOT%/%project%/%module%
SET build=%BUILD_ROOT%/%project%

SET CC=clang++
SET DEFS=-DENGINE_DEBUG_BUILD -DWOLFSSL_TLS13 -D_CRT_SECURE_NO_WARNINGS -DSvcAppPROJECT=\"%project%\" -DSvcAppMODULE=\"%module%\" -DSvcAppBRANCH=\"%branch%\"
SET CCFLAGS=-std=c++17 -Wall -g %DEFS%
SET INCLUDES=-I%ENGINE%/Base/include -I%ENGINE%/Reflection/include -I%ENGINE%/Core/include -I%ENGINE%/ToolCore/include -I%ENGINE%/Network/include -I%ENGINE%/Storage/include -I%ENGINE%/Render/include -I%ENGINE%/Render/include/imgui -I%ENGINE%/BuildInfo/include -I%code%
SET SEARCH_PATHS=-L%BUILD_ROOT%/Engine -L%BUILD_ROOT%/External -L%code%/glfw/lib-vc2022
SET DEPS=-lEngine.Base -lEngine.Core -lEngine.ToolCore -lEngine.Network -lEngine.Reflection -lEngine.Storage -lEngine.Render -lwolfssl -lwpcap -lPacket -lUser32 -lAdvapi32 -lBcrypt

set TARGET=%build%/%module%_%LOCAL_USER_PLATFORM%_%LOCAL_USER_CONFIG%.%ext%

set imgui=%ENGINE%/Render/imgui
set ImGui_s=%imgui%/imgui.cpp %imgui%/imgui_demo.cpp %imgui%/imgui_draw.cpp %imgui%/imgui_tables.cpp %imgui%/imgui_widgets.cpp %imgui%/backends/imgui_impl_glfw.cpp %imgui%/backends/imgui_impl_opengl3.cpp

set SOURCES=%code%/App.cpp %code%/main.cpp %code%/Startup.cpp

mkdir %build:/=\% 2> NUL

IF "%1" NEQ "copylibs" (
    %CC% %CCFLAGS% %SEARCH_PATHS% %DEPS% %INCLUDES% -o %TARGET% %SOURCES%
) else (
    set ERRORLEVEL=0
)

IF %ERRORLEVEL% EQU 0 (
    copy /Y %BUILD_ROOT:/=\%\Engine\Engine.Base.dll %build:/=\% > NUL 2>&1
    copy /Y %BUILD_ROOT:/=\%\Engine\Engine.Core.dll %build:/=\% > NUL 2>&1
    copy /Y %BUILD_ROOT:/=\%\Engine\Engine.Storage.dll %build:/=\% > NUL 2>&1
    copy /Y %BUILD_ROOT:/=\%\Engine\Engine.ToolCore.dll %build:/=\% > NUL 2>&1
    copy /Y %BUILD_ROOT:/=\%\Engine\Engine.Network.dll %build:/=\% > NUL 2>&1
    copy /Y %BUILD_ROOT:/=\%\Engine\Engine.Reflection.dll %build:/=\% > NUL 2>&1
    copy /Y %BUILD_ROOT:/=\%\External\wolfssl.dll %build:/=\% > NUL 2>&1
    copy /Y %BUILD_ROOT:/=\%\External\wpcap.dll %build:/=\% > NUL 2>&1
    copy /Y %BUILD_ROOT:/=\%\External\Packet.dll %build:/=\% > NUL 2>&1
    cls

    IF "%1" NEQ "copylibs" (
        @echo [%project%] %module% has been built. Target: %TARGET%
    ) ELSE (
        @echo Libaries have been copied to %module%'s directory: %build%
    )

    IF "%1" EQU "run" (
        %TARGET%
    )
) ELSE (
    @echo %project%.%module% has failed to build.
)