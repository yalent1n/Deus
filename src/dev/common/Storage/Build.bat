@echo off
set project=Engine
set module=Storage
set ext=dll

SET code=%CODE_ROOT%/%project%/%module%
SET build=%BUILD_ROOT%/%project%

SET CC=clang++
set DEFS=-DENGINE_BUILD_DLL -DENGINE_DEBUG_BUILD -DENGINE_STORAGE_EXPORTS
set CCFLAGS=-std=c++17 -Wall -shared -g %DEFS%
set INCLUDES=-I%CODE_ROOT%/Engine/Base/include -I%CODE_ROOT%/Engine/Core/include -I%CODE_ROOT%/Engine/ToolCore/include -I%CODE_ROOT%/Engine/Network/include -I%code%/include
set SEARCH_PATHS=-L%build%
set DEPS=-lEngine.Base -lEngine.Core -lEngine.ToolCore -lEngine.Network -lAdvapi32 -lbcrypt

set TARGET=%build%/%project%.%module%.%ext%

set Encoding_s=%code%/Delta.cpp %code%/Bulk.cpp
set Deployment_s=%code%/Deployment/BuildLayer.cpp %code%/Deployment/BuildImage.cpp %code%/Deployment/Deploy.cpp
set Pool_s=%code%/StoragePool.cpp %code%/Pools/FileStoragePool.cpp %code%/Pools/FlexibleStoragePool.cpp %code%/LocalChunkStore.cpp
set Io_s=%code%/IoTrustVault.cpp %code%/IoService.cpp %code%/IoStream.cpp
set Cloud_s=%code%/CloudStorageService.cpp
set Svc_s=%code%/Svc/Helix.cpp %code%/Svc/Stream.cpp %code%/Svc/Depot.cpp
set SOURCES=%code%/Bundle.cpp %Encoding_s% %Pool_s% %Io_s% %Deployment_s%

%CC% %CCFLAGS% %SEARCH_PATHS% %DEPS% %INCLUDES% -o %TARGET% %SOURCES%

IF %ERRORLEVEL% EQU 0 (
    cls
    @echo %project%.%module% has been built. Target: %TARGET%
) ELSE (
    @echo %project%.%module% has failed to build.
    exit /b 255
)