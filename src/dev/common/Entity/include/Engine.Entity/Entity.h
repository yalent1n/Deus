#pragma once
#include "EntityBusPeer.h"
#include "EntityOwner.h"

namespace rw
{
class EntityBus;

class Entity : public EntityBusPeer, public EntityOwner
{
public:
    Entity();

    const EntityBus* getEntityBus() const;

private:
    EntityBus* m_entityBus;
};
}