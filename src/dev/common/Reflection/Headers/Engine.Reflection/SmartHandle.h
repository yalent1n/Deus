#pragma once
#include "Bundle.h"
#include "Compartment.h"
#include "Symbol.h"

namespace rw
{
typedef u64 InterpartitionCodec;

struct InterpartitionReference
{
    InterpartitionCodec id;

    FB_REFL_API static InterpartitionReference (*make)(Symbol*);
    FB_REFL_API static Symbol* (*resolve)(InterpartitionReference); // IPR won't save it for faster lookup 
    FB_REFL_API static Symbol* (*resolveAndCache)(InterpartitionReference); // IPR will save it
    FB_REFL_API static void    (*unresolve)(Symbol*); // IPR destroys the symbol in the cache
};

enum : InterpartitionCodec { IsInterpartitionReferenceFlag = 1 };

struct RawIpr
{
    Guid partition;
    Guid instance;
};

/* 
    Holds a reference to any "object" in a given symbol pool.
    A symbol should have a reference to a:
        - compartment (storage), 
        - document (location)
        - and identifier.

    - Every data schema defines it's own document and identifer limit.
    - A symbol instance should be aligned on at least 4 bytes.
*/
template<bool Cached>
class SymbolHandle
{
public:
    // asserts if the symbol is unreachable (eg. inside garbage compartment)
    // or internally invalid
    SymbolHandle(Symbol*);

    // asserts if the symbol is unresolvable (eg. data flags)
    SymbolHandle(InterpartitionReference); 
    ~SymbolHandle();

    SymbolHandle(const SymbolHandle&);
    SymbolHandle(SymbolHandle&&);

    SymbolHandle& operator=(Symbol*);
    SymbolHandle& operator=(InterpartitionReference);
    SymbolHandle& operator=(const SymbolHandle&);
    SymbolHandle& operator=(SymbolHandle&&);

    InterpartitionReference getPacked() const
        { return m_handle; }

    void setPacked(InterpartitionReference ref)
        { m_handle = ref; }

    Symbol* get();
    const Symbol* get() const;

    Symbol* operator->()
        { return get(); }
    
    const Symbol* operator->() const
        { return get(); }

private:
    InterpartitionReference m_handle;
};

template<bool C>
SymbolHandle<C>::SymbolHandle(Symbol* data)
    : m_handle { InterpartitionCodec(data) }
{
    if(Symbol* ptr = get())
    {
        ptr->retain();
    }
}

template<bool C>
SymbolHandle<C>::SymbolHandle(InterpartitionReference ref)
    : m_handle(ref)
{
    if(Symbol* ptr = get())
    {
        ptr->retain();
    }
}

template<bool C>
SymbolHandle<C>::~SymbolHandle()
{
    if(Symbol* sym = get())
    {
        uint refcnt = sym->detach();
        if(!refcnt)
        {
            m_handle.id = 0;

            if(sym->isUnreferenced())
                return;
                
            delete sym;
        }
    }    
}

template<bool C>
SymbolHandle<C>::SymbolHandle(const SymbolHandle& other)
    : m_handle(other.m_handle)
{}

template<bool C>
SymbolHandle<C>::SymbolHandle(SymbolHandle&& other)
    : m_handle(other.m_handle)
{
    other.m_handle.id = 0;
}
template<bool C>
SymbolHandle<C>& SymbolHandle<C>::operator=(Symbol* data)
{
    m_handle = { InterpartitionCodec(data) };
    return *this;
}

template<bool C>
SymbolHandle<C>& SymbolHandle<C>::operator=(InterpartitionReference ref)
{
    m_handle = ref;
    return *this;
}

template<bool C>
SymbolHandle<C>& SymbolHandle<C>::operator=(const SymbolHandle& rhs)
{
    m_handle = rhs.m_handle;
    return *this;
}

template<bool C>
SymbolHandle<C>& SymbolHandle<C>::operator=(SymbolHandle&& rhs)
{
    m_handle = rhs.m_handle;
    rhs.m_handle.id = 0;
    return *this;
}

template<bool C>
Symbol* SymbolHandle<C>::get()
{
    if(m_handle.id & IsInterpartitionReferenceFlag)
    {
        // at least one lookup will be performed anyway
        if constexpr (C)
            return InterpartitionReference::resolveAndCache(m_handle);
    
        return InterpartitionReference::resolve(m_handle);
    }

    return reinterpret_cast<Symbol*>(m_handle.id);
}

template<bool C>
const Symbol* SymbolHandle<C>::get() const
{
    return get();    
}

template<class T, bool Cached>
class SmartHandle : public SymbolHandle<Cached>
{
public:
    typedef SymbolHandle<Cached> Base_t;

    SmartHandle(T* data)
        : Base_t(data)
    {}
    
    SmartHandle(InterpartitionReference ref)
        : Base_t(ref)
    {}    

    SmartHandle(const SmartHandle& other)
        : Base_t(reinterpret_cast<const SymbolHandle<Cached>&>(other))
    {}

    SmartHandle(SmartHandle&& other)
        : Base_t(reinterpret_cast<SymbolHandle<Cached>&&>(other))
    {}

    SmartHandle& operator=(T* dataT)
    {
        Base_t::operator=(reinterpret_cast<Symbol*>(dataT));
        return *this;
    }

    SmartHandle& operator=(InterpartitionReference ref)
    {
        Base_t::operator=(ref);
        return *this;
    }

    SmartHandle& operator=(const SmartHandle<T>& rhs)
    {
        Base_t::operator=(reinterpret_cast<const SymbolHandle<Cached>&>(rhs));
        return *this;
    }

    SmartHandle& operator=(SmartHandle&& rhs)
    {
        Base_t::operator=(reinterpret_cast<SymbolHandle<Cached>&&>(rhs));
        return *this;
    }

    T* get() { return static_cast<T*>(Base_t::get()); }
    const T* get() const { return static_cast<T*>(Base_t::get()); }
    T* operator->() { return get(); }
    const T* operator->() const { return get(); }
};

template<class T>
using UncachedSmartHandleBase = SymbolHandle<false>;

template<class T>
using UncachedSmartHandle = SmartHandle<T, false>;
}