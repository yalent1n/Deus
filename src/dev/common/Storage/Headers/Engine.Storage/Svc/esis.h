#pragma once
#include "../Bundle.h"
#include <Engine.Base/Misc/Time.h>
#include <Engine.Core/FsBackend.h>

// - Synthesis
namespace rw::esis
{
typedef uint Permission;
enum : Permission
{    
    /*
        File & stream permissions
    */
    PERM_SYNC_DOWN = _BIT(1), // queue changes till a branch sync arrives, then create an oplog on-demand
    PERM_COPY = _BIT(2), // branch: FORK BRANCH, file: COPY FILE 
    PERM_SYNC_UP = _BIT(3), // queue changes till a branch sync arrives, then create an oplog on-demand
    PERM_CREATE = _BIT(4), // branch: CREATE BRANCH, file: CREATE FILE
    PERM_DELETE = _BIT(5), // branch: DELETE BRANCH, file: DELETE FILE
    PERM_RENAME = _BIT(6), // branch: RENAME BRANCH, file: RENAME FILE
    PERM_LOCKIO = _BIT(7) // branch: LOCK IO, file: LOCK WRITE
};

static inline constexpr Permission permission_suites[] = {
    /* release */   PERM_SYNC_DOWN | PERM_COPY | PERM_LOCKIO,
    /* mainline */  PERM_SYNC_DOWN | PERM_COPY | PERM_LOCKIO | PERM_SYNC_UP,
    /* feature */   PERM_SYNC_DOWN | PERM_COPY | PERM_LOCKIO | PERM_SYNC_UP | PERM_CREATE,
    /* virtual */   PERM_COPY | PERM_LOCKIO | PERM_SYNC_UP | PERM_CREATE | PERM_DELETE
};

// A development break's stages are defined in streams.
struct StreamSpec
{
    enum Type : u32
    {
        RELEASE, // represents a shipment
        MAINLINE, // represents a primary stream
        FEATURE, // represents a secondary stream
        VIRTUAL, // can be filtered from any stream (temporary)
        MAXTYPES
    };

    enum Direction : u32
    {
        LEFT,
        UP,
        RIGHT,
        DOWN,
        MAXDIRECTIONS
    };

    const char* name;
    uint changes; // initialized on sync down (sync: same as head)
    Type type;
    Permission suite;
    StreamSpec* neighbours[MAXDIRECTIONS];

    bool validate()
    {
        switch(type)
        {
            case RELEASE:
                return suite == permission_suites[RELEASE] && (neighbours[DOWN] && neighbours[DOWN]->validate());
            case MAINLINE:
                return suite == permission_suites[MAINLINE] && (neighbours[DOWN] && neighbours[DOWN]->validate()) && (neighbours[UP] && neighbours[UP]->validate());
            case FEATURE:
                return suite == permission_suites[FEATURE] && (neighbours[DOWN] && neighbours[DOWN]->validate()) && (neighbours[UP] && neighbours[UP]->validate());
            case VIRTUAL:
                return suite == permission_suites[VIRTUAL] && (neighbours[UP] && neighbours[UP]->validate());
            default:
                return false;
        }
    }
};

struct DomainSpec
{
    const char* name;
    const char* long_name;
    StreamSpec* streams[StreamSpec::MAXTYPES];
    DateTime access_time, modified_time, creation_time;
};
}