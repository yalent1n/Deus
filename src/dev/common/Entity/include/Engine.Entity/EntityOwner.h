#pragma once
#include <Engine.Base/Types.h>

namespace rw 
{ 
typedef u64 EntityUid; // Universal identifier

class EntityOwner 
{
public:
    EntityOwner();

    EntityUid getUid() const;

private:
    EntityUid m_uid;
    EntityOwner* m_next;
};
}