#include <Engine.Reflection/Compartment.h>
#include <Engine.Base/Memory/VirtualAlloc.h>

namespace rw
{
struct RCData
{
    /* compartment information */
    const char* name = NIL;
    size_t size = 0;

    /* heap information */
    HHEAP heap = NIL;
    u8 heapData[HEAP_OBJECT_LENGTH] {0};
    RCData* proxyRc;

    RCData() = default;
};

static RCData s_compartments[MAXCOMPARTMENTS];

void initializeCompartment(RCData* self, const char* name, size_t size, Compartment proxy)
{
    self->name = name;
    self->size = size;
    self->proxyRc = &s_compartments[proxy];
    self->heap = &self->heapData[0];
    
    DWORD dwResult = CreateHeapObject(self->heap, size);
    EASSERTF(dwResult, "Failed to create heap for compartment \"%s\"", self->name);

#if defined(ESIS_DEBUG_COMPARTMENT_LOAD)
    Logf("[engine] Loaded new compartment %s", name);
#endif
}

void deinitializeCompartment(RCData* self)
{
    DWORD dwResult = DestroyHeapObject(self->heap);
    EASSERTF(dwResult, "Failed to destroy heap for compartment \"%s\"", self->name);

#if defined(ESIS_DEBUG_COMPARTMENT_LOAD)
    Logf("[engine] Unloaded new compartment %s", self->name);
#endif
}

ResourceCompartment::ResourceCompartment(Compartment compartment, const char* name, size_t size, Compartment proxy)
    : m_compartment(compartment)
{    
    RCData* self = &s_compartments[compartment];
    if(!self->heap)
    {
        initializeCompartment(self, name, size, proxy);
    }
}

ResourceCompartment::ResourceCompartment(Compartment compartment)
    : m_compartment(compartment)
{}

ResourceCompartment::~ResourceCompartment()
{
    deinitializeCompartment(&s_compartments[m_compartment]);
}

ResourceCompartmentHandle ResourceCompartment::getHandle(Compartment c)
{
    RCData* self = NIL;
    switch(c)
    {
        case Compartment_ACTIVE:
            self = s_compartments[Compartment_ACTIVE].proxyRc;
            break;
        case Compartment_GARBAGE:
            self = s_compartments[Compartment_GARBAGE].proxyRc;
            break;
        case Compartment_FORBIDDEN:
            self = s_compartments[Compartment_FORBIDDEN].proxyRc;
            break;
        default:
            self = &s_compartments[c];
            break;
    }

    if(self && self->heap)
    {
        return static_cast<ResourceCompartmentHandle>(self);
    }

    return nullptr;
}

ResourceCompartmentAllocator ResourceCompartment::getAllocator(ResourceCompartmentHandle h)
{
    if(!h)
        return NIL;

    return ((RCData*)h)->heap;
}

void ResourceCompartment::linkActive(Compartment c)
{
    EASSERT(c != Compartment_ACTIVE, "Invalid compartment to set as active (Compartment_ACTIVE)");
    RCData* self = &s_compartments[c];
    EASSERTF(self->heap, "Compartment %u is uninitialized (failed to set it as active)", c);
    s_compartments[Compartment_ACTIVE].proxyRc = self;
#if defined(ENGINE_DEBUG_SPECIAL_COMPARTMENTS)
    Logf("[engine] Compartment %u has been marked as active for in-use resources", c);
#endif
}

void ResourceCompartment::linkGarbage(Compartment c)
{
    EASSERT(c != Compartment_GARBAGE, "Invalid compartment to set as garbage (Compartment_GARBAGE)");
    RCData* self = &s_compartments[c];
    EASSERTF(self->heap, "Compartment %u is uninitialized (failed to set it as garbage)", c);
    s_compartments[Compartment_GARBAGE].proxyRc = self;
#if defined(ENGINE_DEBUG_SPECIAL_COMPARTMENTS)
    Logf("[engine] Compartment %u has been marked as garbage for unreferenced resources", c);
#endif
}

void ResourceCompartment::linkForbidden(Compartment c)
{
    EASSERT(c != Compartment_FORBIDDEN, "Invalid compartment to set as forbidden (Compartment_FORBIDDEN)");
    RCData* self = &s_compartments[c];
    EASSERTF(self->heap, "Compartment %u is uninitialized (failed to set it as forbidden)", c);
    s_compartments[Compartment_FORBIDDEN].proxyRc = self;
#if defined(ENGINE_DEBUG_SPECIAL_COMPARTMENTS)
    Logf("[engine] Compartment %u has been marked as forbidden for unhandled resources", c);
#endif
}

void ResourceCompartment::unlinkActive()
{
    s_compartments[Compartment_ACTIVE].proxyRc = NIL;
}

void ResourceCompartment::unlinkGarbage()
{
    s_compartments[Compartment_GARBAGE].proxyRc = NIL;
}

void ResourceCompartment::unlinkForbidden()
{
    s_compartments[Compartment_FORBIDDEN].proxyRc = NIL;
}
}