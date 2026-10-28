#pragma once
#include <Engine.Base/Types.h>

namespace rw
{
typedef u32 PeerId;

class EntityBusPeer
{
public:
    EntityBusPeer();

    PeerId getId() const;

private:
    PeerId m_peerId;
    u32 m_refCount;
};
}