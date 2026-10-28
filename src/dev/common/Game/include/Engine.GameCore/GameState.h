#pragma once
#include "Bundle.h"
#include <Engine.Base/Database/DbOid.h>

namespace rw
{
/*
*** Game State ***
Maintains a centralized multi-level state with multiplexing and channels.
******************

----------    STREAM    ----------
| SERVER |    ----->    | CLIENT |
----------              ----------
*/

class GameStateClientStream;
class GameStateServerStream;

class GSClient;
class GSServer;

enum GameStateStreamType : u8
{
    GSSType_Level,
    GSSType_World,
    GSSType_Player,
    GSSType_Physics,
    GSSType_Custom
};

struct GameStateStream
{
    GameStateStreamType Type;
    u32 ID;
    u32 SizeOfPeerTable;

    DbOid Peers[1];
};

namespace gsPackets
{
    struct C00HelloPacket
    {
        u32 Version;
        DbOid PeerID;
    };

    struct S00WelcomePacket
    {
        u32 Version;
        u32 SizeOfStreamTable;
        GameStateStream Streams[1];
    };
}
}