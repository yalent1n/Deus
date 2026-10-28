@echo off
set project=Engine
set module=GameCore
set ext=dll

SET code=%CODE_ROOT%/%project%/%module%
SET build=%BUILD_ROOT%/%project%

SET CC=clang++
set DEFS=-DENGINE_BUILD_DLL -DFB_BUILDING_GAMECORE_MODULE
set CCFLAGS=-std=c++17 -Wall -shared -g %DEFS%
set INCLUDES=-I%CODE_ROOT%/Engine/Base/include -I%CODE_ROOT%/Engine/Core/include -I%CODE_ROOT%/Engine/ToolCore/include -I%CODE_ROOT%/Engine/Storage/include -I%code%/include
set SEARCH_PATHS=-L%build%
set DEPS=-lEngine.Base -lEngine.Core -lEngine.ToolCore -lEngine.Storage -lAdvapi32

set TARGET=%build%/%project%.%module%.%ext%
set SOURCES=

%CC% %CCFLAGS% %SEARCH_PATHS% %DEPS% %INCLUDES% -o %TARGET% %SOURCES%

IF %ERRORLEVEL% EQU 0 (
    cls
    @echo %project%.%module% has been built. Target: %TARGET%
) ELSE (
    @echo %project%.%module% has failed to build.
    exit /b 255
)