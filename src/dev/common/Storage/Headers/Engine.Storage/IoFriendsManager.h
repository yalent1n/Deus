#pragma once
#include "Bundle.h"

namespace rw
{
inline constexpr size_t DGSC_UNIQUE_KEY_SIZE = 64;

struct UniqueKey
{
    u8 bytes[DGSC_UNIQUE_KEY_SIZE];
};

struct IdentityNode
{
    
};

interface IIdentityManager
{
    nodefine bool Add(const UniqueKey& key) = 0;
    nodefine bool Remove(const UniqueKey& key) = 0;
    nodefine bool Block(const UniqueKey& key) = 0;
    nodefine bool Unblock(const UniqueKey& key) = 0;

    nodefine bool HasRelationshipWith(const UniqueKey& key) = 0;
    nodefine bool HasBeenBlocked(const UniqueKey& key) = 0;
};

class DgscIdentityManager : public IIdentityManager
{

};

FB_STORAGE_API IIdentityManager* _DgscIdentityManager();
}