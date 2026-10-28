#include <Engine.Reflection/SmartHandle.h>

namespace rw
{
InterpartitionReference (*InterpartitionReference::make)(Symbol*);
Symbol* (*InterpartitionReference::resolve)(InterpartitionReference);
Symbol* (*InterpartitionReference::resolveAndCache)(InterpartitionReference);
void (*InterpartitionReference::unresolve)(Symbol*);
}