#pragma once
#include "EntityOwner.h"

namespace rw
{
class Entity;

// A collection of entities (which some of them can be a ComponentEntity)
class EntityBus : public EntityOwner
{
public:
    EntityBus();

private:
    Entity* m_entities;
    
};
}