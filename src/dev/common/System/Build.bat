@echo off
set bundle_name=CoreFramework
set bundle_id=com.konsant.%bundle_name%
set bundle_type=framework

SET code=%CODE_ROOT%%bundle_id%
SET build=%BUILD_ROOT%%bundle_name%.%bundle_type%

SET CC=clang++
set DEFS=-DENGINE_BUILD_DLL -DENGINE_CORE_EXPORTS -DENGINE_DEBUG_BUILD -D_CRT_SECURE_NO_WARNINGS
set CCFLAGS=-std=c++17 -Wall -shared -g %DEFS%
set INCLUDES=-I%CODE_ROOT%/com.konsant.CorePrivate/Headers -I%code%/Headers
set SEARCH_PATHS=-L%BUILD_ROOT%CorePrivate.Framework/Binaries/%LOCAL_USER_CONFIG%/%LOCAL_USER_PLATFORM%
set DEPS=-lCorePrivate -lAdvapi32

set TARGET=%build%/Binaries/%LOCAL_USER_CONFIG%/%LOCAL_USER_PLATFORM%/%bundle_name%

set Base_s=%code%/VirtualFileSystem.cpp %code%/FsBackend.cpp
set diag_s=%code%/diag/ExecutionContext.cpp %code%/diag/Environment.cpp
set Buffer_s=%code%/Buffer/Win32/Win32Buffer.cpp
set Zip_s=%code%/Zip/ZipIo.cpp %code%/Zip/ZipFormat.cpp
set Backend_s=%code%/Backends/RegistryIoBackend.cpp %code%/Backends/Win32IoBackend.cpp

set SOURCES=%code%/ModuleInfo.cpp %code%/Bundle.cpp %Base_s% %diag_s% %Buffer_s% %Backend_s% %Zip_s%

%CC% %CCFLAGS% %SEARCH_PATHS% %DEPS% %INCLUDES% -o %TARGET% %SOURCES%

IF %ERRORLEVEL% EQU 0 (
    cls
    @echo %bundle_id% has been built. Target: %TARGET%
) ELSE (
    @echo %bundle_id% has failed to build.
    exit /b 255
)