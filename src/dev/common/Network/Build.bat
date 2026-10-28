@echo off
set project=Engine
set module=Network
set ext=dll

SET code=%CODE_ROOT%/%project%/%module%
SET build=%BUILD_ROOT%/%project%

SET CC=clang++
set DEFS=-DENGINE_BUILD_DLL -DENGINE_DEBUG_BUILD -DENGINE_NETWORK_EXPORTS -D_WINSOCK_DEPRECATED_NO_WARNINGS -DWOLFSSL_TLS13 -DWC_NO_HARDEN
set CCFLAGS=-std=c++17 -Wall -shared -g %DEFS%
set INCLUDES=-I%CODE_ROOT%/Engine/Base/include -I%CODE_ROOT%/Engine/Core/include -I%code%/include
set SEARCH_PATHS=-L%build% -L%BUILD_ROOT%/External
set DEPS=-lEngine.Base -lEngine.Core -lwolfssl -lwpcap -lPacket -lAdvapi32

set TARGET=%build%/%project%.%module%.%ext%

set NetBase_s=%code%/Bundle.cpp %code%/HttpSession.cpp %code%/SocketBase.cpp %code%/Util.cpp %code%/Netcrypt.cpp
set NetConn_s=%code%/TcpClientConnection.cpp %code%/TcpServer.cpp %code%/TcpServerConnection.cpp %code%/TcpServerConnectionImpl.cpp %code%/Relay.cpp

set http=%code%/http
set Http_s=%http%/base.cpp %http%/server.cpp

set awdl=%code%/mesh80211
set Awdl_s=%awdl%/frame.cpp %awdl%/schedule.cpp %awdl%/sync.cpp %awdl%/atlas.cpp

set SOURCES=%NetBase_s% %NetConn_s% %NetRelay_s% %Awdl_s% %Http_s%

mkdir %BinDir:/=\\% 2> NUL
%CC% %CCFLAGS% %SEARCH_PATHS% %DEPS% %INCLUDES% -o %TARGET% %SOURCES%

IF %ERRORLEVEL% EQU 0 (
    cls
    @echo %project%.%module% has been built. Target: %TARGET%
) ELSE (
    @echo %project%.%module% has failed to build.
    exit /b 255
)