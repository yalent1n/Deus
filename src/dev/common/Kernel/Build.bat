@echo off
set bundle_name=CorePrivate
set bundle_id=com.konsant.%bundle_name%
set bundle_type=framework

set EXTERNAL_BINARIES=%BUILD_ROOT%External
set EXTERNAL_SOURCES=-I%CODE_ROOT%External/compr-1.0.0

SET code=%CODE_ROOT%%bundle_id%
SET build=%BUILD_ROOT%%bundle_name%.%bundle_type%

SET CC=clang++
SET DEFS=-DENGINE_BUILD_DLL -DENGINE_BASE_EXPORTS -DENGINE_DEBUG_BUILD
SET CCFLAGS=-std=c++17 -Wall -shared -g %DEFS%
SET INCLUDES=-I%code%/Headers %EXTERNAL_SOURCES%
SET SEARCH_PATHS=-L%EXTERNAL_BINARIES%
SET DEPS=-lAdvapi32 -lBcrypt -llz4 -lzlib -lzstd

set TARGET=%build%/Binaries/%LOCAL_USER_CONFIG%/%LOCAL_USER_PLATFORM%/%bundle_name%

set Base_s=%code%/Config.cpp %code%/Log.cpp %code%/MIME.cpp %code%/RegistryKey.cpp
set Buffer_s=%code%/Buffer/SharedBuffer.cpp %code%/Buffer/Buffer.cpp %code%/Buffer/Adapter.cpp %code%/Buffer/RegistryBuffer.cpp
set Stream_s=%code%/Buffer/Stream/EndianInStream.cpp
set Database_s=%code%/Database/DbData.cpp %code%/Database/DbOid.cpp %code%/Database/DbObject.cpp %code%/Database/DbObjectBuilder.cpp
set Memory_s=%code%/Memory/Util.cpp %code%/Memory/Map.cpp %code%/Memory/VirtualAlloc.cpp %code%/Memory/Allocator.cpp %code%/Memory/LinearAllocator.cpp %code%/Memory/PageAllocator.cpp %code%/Memory/HeapAllocator.cpp
set Thread_s=%code%/Thread/CriticalSection.cpp %code%/Thread/Thread.cpp %code%/Thread/WorkerThreadPool.cpp
set String_s=%code%/String/Util.cpp %code%/String/StringBuilder.cpp %code%/String/eastl_string.cpp
set Crypto_s=%code%/Crypto/SHAFunctions.cpp %code%/Crypto/SHA.cpp %code%/Crypto/Base64.cpp %code%/Crypto/Guid.cpp %code%/Crypto/AES.cpp %code%/Crypto/HKDF.cpp %code%/Crypto/ECDH.cpp
set Misc_s=%code%/Misc/Time.cpp %code%/Misc/Vars.cpp %code%/Misc/Util.cpp %code%/Misc/Event.cpp %code%/Misc/Json.cpp %code%/Misc/Xml.cpp %code%/Misc/Ini.cpp %code%/Misc/gzip.cpp %code%/Misc/Codec.cpp %code%/Misc/Interlocked.cpp

set SOURCES=%Base_s% %code%/Bundle.cpp %Thread_s% %String_s% %Buffer_s% %Memory_s% %Crypto_s% %Misc_s% %Database_s%

%CC% %CCFLAGS% %SEARCH_PATHS% %DEPS% %INCLUDES% -o %TARGET% %SOURCES%

IF %ERRORLEVEL% EQU 0 (
    cls
    @echo %bundle_id% has been built. Target: %TARGET%
) ELSE (
    @echo %bundle_id% has failed to build.
)