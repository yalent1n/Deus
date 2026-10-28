#pragma once
#include <Engine.Core/Bundle.h>
#include "SymbolClassInfo.h"
#include "SymbolMacros.h"
#include "Array.h"

namespace rw
{
interface DatabasePartition;

class Symbol : public Object
{
    friend class SymbolClassInfo;
    DECLARE_ABSTRACT_SYMBOL(Core, Symbol, Symbol)

public:
    uint getRetainCount() const
        { return m_refCount; }

    Compartment getCompartment()
        { return m_compartment; }

    const SymbolClassInfo* getType() const override final 
        { return (const SymbolClassInfo*) m_typeInfo; }

public:
    FB_REFL_API  Symbol();
    FB_REFL_API ~Symbol();

    REFL_TYPEINFO_MEMBERTYPE(kTypeCode_Void, kTypeCategory_Class, MemberInfoFlags::ABSTRACT)    

    FB_REFL_API uint retain();
    FB_REFL_API uint detach();

    FB_REFL_API Compartment getCompartment() const;

    FB_REFL_API const Guid* getInstanceGuid() const;
    FB_REFL_API void setInstanceGuid(const Guid& guid);

    FB_REFL_API DatabasePartition* getPartition() const;
    FB_REFL_API void setPartition(DatabasePartition* pool);

    FB_REFL_API bool isAggregated() const;
    FB_REFL_API void setAggregated(bool state);
    
    FB_REFL_API bool isTraceable() const;
    FB_REFL_API void setTraceable(bool state);
    
    FB_REFL_API bool isExported() const;
    FB_REFL_API void setExported(bool state);
    
    FB_REFL_API bool isUnresolvable() const;
    FB_REFL_API void setUnresolvable(bool state);
    
    FB_REFL_API bool isUnreferenced() const;
    FB_REFL_API void setUnreferenced(bool state);

private:
    typedef uint SymbolFlags;
    enum : SymbolFlags
    {
        S_F_AGGREGATED = 1 << 0,     // in a partition
        S_F_TRACEABLE = 1 << 1,      // an object tracer can find the instance by guid
        S_F_EXPORTED = 1 << 2,       // has an instance GUID
        S_F_UNREFERENCED = 1 << 3,   // finally all references are dropped, ready to be freed (IF NOT IN A PARTITION)
        S_F_UNRESOLVABLE = 1 << 4,   // no IPR can resolve this object
    };

protected:
    volatile uint m_refCount;
    SymbolFlags m_flags;
    Compartment m_compartment;
};

struct OffsetToFirstField
{
    static inline constexpr u32 value = 
//#if defined(__clang__)
#if 0
		__builtin_offsetof(Symbol, m_compartment) + sizeof(Compartment);
#else
		sizeof(Symbol); // seems MS platforms properly incorporate padding when subclassing.
#endif
};
}