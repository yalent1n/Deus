@echo off
set project=Engine
set module=ToolCore
set ext=dll

SET code=%CODE_ROOT%/%project%/%module%
SET build=%BUILD_ROOT%/%project%

SET CC=clang++
set DEFS=-DENGINE_BUILD_DLL -DENGINE_TOOLCORE_EXPORTS
set CCFLAGS=-std=c++17 -Wall -shared -g %DEFS%
set INCLUDES=-I%CODE_ROOT%/Engine/Base/include -I%CODE_ROOT%/Engine/Core/include -I%code%/include
set SEARCH_PATHS=-L%build%
set DEPS=-lEngine.Base -lEngine.Core -lAdvapi32

set TARGET=%build%/%project%.%module%.%ext%

set Base_s=%code%/CompressionHelper.cpp
set App_s=%code%/App/ServiceBase.cpp %code%/App/ApplicationBase.cpp

set SOURCES=%code%/Bundle.cpp %Base_s% %App_s%

mkdir %build:/=\\% 2> NUL
%CC% %CCFLAGS% %SEARCH_PATHS% %DEPS% %INCLUDES% -o %TARGET% %SOURCES%

IF %ERRORLEVEL% EQU 0 (
    cls
    @echo %project%.%module% has been built. Target: %TARGET%
) ELSE (
    @echo %project%.%module% has failed to build.
    exit /b 255
)