@echo off
set project=Engine
set module=Reflection
set ext=dll

SET code=%CODE_ROOT%/%project%/%module%
SET build=%BUILD_ROOT%/%project%

SET CC=clang++
set DEFS=-DENGINE_BUILD_DLL -DENGINE_REFLECTION_EXPORTS -DENGINE_DEBUG_BUILD -D_CRT_SECURE_NO_WARNINGS
set CCFLAGS=-std=c++17 -Wall -shared -g %DEFS%
set INCLUDES=-I%CODE_ROOT%/Engine/Base/include -I%CODE_ROOT%/Engine/Core/include -I%code%/include
set SEARCH_PATHS=-L%build%
set DEPS=-lEngine.Base -lEngine.Core -lAdvapi32

set TARGET=%build%/%project%.%module%.%ext%
set Base_s=%code%/Compartment.cpp %code%/TypeInfo.cpp %code%/ArrayTypeInfo.cpp %code%/PrimitiveTypeInfo.cpp %code%/ValueTypeInfo.cpp %code%/ClassInfo.cpp
set Type_s=%code%/NativeTypeRegistry.cpp %code%/ArrayBase.cpp
set Symbol_s=%code%/SymbolClassInfo.cpp %code%/Symbol.cpp %code%/SmartHandle.cpp %code%/Asset.cpp
set ScriptEngine_s=%code%/ScriptEngine/mm.cpp %code%/ScriptEngine/image.cpp
set Runtime_s=%code%/RuntimeIpr.cpp %code%/RuntimeDatabasePartition.cpp
set Partition_s=%code%/GenericDataLayout.cpp %Runtime_s%
set SOURCES=%code%/Bundle.cpp %Base_s% %Type_s% %Symbol_s% %ScriptEngine_s% %Partition_s%

%CC% %CCFLAGS% %SEARCH_PATHS% %DEPS% %INCLUDES% -o %TARGET% %SOURCES%

IF %ERRORLEVEL% EQU 0 (
    cls
    @echo %project%.%module% has been built. Target: %TARGET%
) ELSE (
    @echo %project%.%module% has failed to build.
    exit /b 255
)