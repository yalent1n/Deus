#pragma once
#include "Bundle.h"

namespace rw
{
inline constexpr uint COMPARTMENT_BITS = 7;

/*  NOTE

    Default Compartments:
        - Static: Used allocated objects
        - Garbage: Unused allocated objects waiting for deletion

    Dynamic Compartments:
        - The compartment manager (TODO) will find the first free
        dynamic compartment slot and release it later.

    Active compartment:
        - Redirects actions to the currently used dynamic compartment (fallback to forbidden)
*/

enum Compartment : u8
{
    Compartment_STATIC_Begin,
    Compartment_STATIC_End = Compartment_STATIC_Begin + _MASK(1),
    
    Compartment_DYNAMIC_Begin,
    Compartment_DYNAMIC_End = Compartment_DYNAMIC_Begin + _MASK(7),
    
    // Forbidden compartment, unknown use
    Compartment_FORBIDDEN,

    // Number of usable compartments (additional invalid)
    MAXCOMPARTMENTS = Compartment_FORBIDDEN + 1,

    // Invalid compartment, useful for syntax and management
    Compartment_INVALID = Compartment_STATIC_Begin,

    // Static compartment, lives as long as the application  (default)
    Compartment_STATIC = Compartment_STATIC_End,

    // Proxy compartment, redirects to the active compartment
    Compartment_ACTIVE = Compartment_DYNAMIC_Begin,
    
    // Garbage compartment, unused symbols arrive here (for later release)
    Compartment_GARBAGE = Compartment_DYNAMIC_End,
};

typedef void* ResourceCompartmentHandle;
typedef HHEAP ResourceCompartmentAllocator;

// Domains use resource compartments for storing symbol data.
class ResourceCompartment final
{
public:
    ResourceCompartment() = default;

    // Creates a new resource compartment with an optional proxy.
    FB_REFL_API ResourceCompartment(
        Compartment compartment,
        const char* name,
        size_t size,
        Compartment proxy = Compartment_INVALID
    );
    FB_REFL_API ~ResourceCompartment();

private:
    explicit ResourceCompartment(Compartment);

public:
    Compartment getCompartment() const
        { return m_compartment; }
    
    // Returns the compartment handle at the specified slot.
    FB_REFL_API static ResourceCompartmentHandle getHandle(Compartment c);
    FB_REFL_API static ResourceCompartmentAllocator getAllocator(ResourceCompartmentHandle h);

    FB_REFL_API static void linkActive(Compartment c);
    FB_REFL_API static void linkGarbage(Compartment c);
    FB_REFL_API static void linkForbidden(Compartment c);

    FB_REFL_API static void unlinkActive();
    FB_REFL_API static void unlinkGarbage();
    FB_REFL_API static void unlinkForbidden();
private:    
    Compartment m_compartment;
};
}