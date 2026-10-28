#include <Engine.Reflection/Symbol.h>
#include <Engine.Base/Misc/Interlocked.h>

namespace rw
{
BEGIN_FIELDS_EXPORT(Symbol)
END_FIELDS_EXPORT()

BEGIN_TYPE_ATTRIBUTES_EXPORT(Symbol)
END_ATTRIBUTES_EXPORT()

EXPORT_ABSTRACT_SYMBOL(Symbol)

Symbol::Symbol()
{
    ELOG("new symbol data");
}

Symbol::~Symbol()
{
    ELOG("deleted symbol data");
}

uint Symbol::retain()
{
    return lock_inc(m_refCount);
}

uint Symbol::detach()
{
    if(m_flags & S_F_UNREFERENCED)
    {
        return 0;
    }

    uint cnt = lock_dec(m_refCount);
    if(!cnt)
    {
        m_flags |= S_F_UNREFERENCED;
        getType()->deinitializeSymbol(this);
        return 0;
    }

    return cnt;
}

#define Symbol_Slot(index) (((u64*)this) - (index))
#define Symbol_Slot_As(typ, index) (*(typ*)(((u64*)this) - (index)))

const Guid* Symbol::getInstanceGuid() const
{
    if(m_flags & S_F_EXPORTED)
        return &Symbol_Slot_As(const Guid, SymbolClassInfo::c_guidEntryIndex);

    return nullptr;
}

void Symbol::setInstanceGuid(const Guid& guid)
{
    if(m_flags & S_F_EXPORTED)
    {
        Symbol_Slot_As(Guid, SymbolClassInfo::c_guidEntryIndex) = guid;
    }
}

DatabasePartition* Symbol::getPartition() const
{
    if(m_flags & S_F_EXPORTED)
        return Symbol_Slot_As(DatabasePartition*, SymbolClassInfo::c_partitionEntryIndex);

    return nullptr;
}

void Symbol::setPartition(DatabasePartition* pool)
{
    if(m_flags & S_F_AGGREGATED)
    {
        Symbol_Slot_As(DatabasePartition*, SymbolClassInfo::c_partitionEntryIndex) = pool;
    }
}

#define ImplementFlagSwitch(flag) m_flags = (state ? (m_flags | (flag)) : (m_flags & (~(flag))))

bool Symbol::isAggregated() const
{
    return !!(m_flags & S_F_AGGREGATED);
}

void Symbol::setAggregated(bool state)
{
    ImplementFlagSwitch(S_F_AGGREGATED);
}

bool Symbol::isTraceable() const
{
    return !!(m_flags & S_F_TRACEABLE);
}

void Symbol::setTraceable(bool state)
{
    ImplementFlagSwitch(S_F_TRACEABLE);
}

bool Symbol::isExported() const
{
    return !!(m_flags & S_F_EXPORTED);
}

void Symbol::setExported(bool state)
{
    ImplementFlagSwitch(S_F_EXPORTED);
}

bool Symbol::isUnresolvable() const
{
    return !!(m_flags & S_F_UNRESOLVABLE);
}

void Symbol::setUnresolvable(bool state)
{
    ImplementFlagSwitch(S_F_UNRESOLVABLE);
}

bool Symbol::isUnreferenced() const
{
    return !!(m_flags & S_F_UNREFERENCED);
}

void Symbol::setUnreferenced(bool state)
{
    ImplementFlagSwitch(S_F_UNREFERENCED);
}
}