#pragma once
#include "Ref.h"
#include "Memory/VirtualAlloc.h"
#include "Misc/Interlocked.h"

namespace rw
{
// A reference counted object which is freed when goes out of scope. Not thread safe!
template<class T>
class SmartRef
{
public:
    SmartRef() = default;
    SmartRef(T* ptr)
        : m_ptr(ptr)
    {
        if(ptr)
            addRef();
    }
    SmartRef(const SmartRef& other) 
        : m_ptr(other.m_ptr)
        , m_refCount(other.m_refCount)
    {
        addRef();
    }
    SmartRef(SmartRef&& other) 
        : m_ptr(other.m_ptr) 
        , m_refCount(other.m_refCount)
    { 
        other.m_ptr = nullptr;
        other.m_refCount = 0;
    }
    ~SmartRef() { release(); }

    SmartRef& operator=(T* ptr)
    {
        m_ptr = ptr;
        if(ptr)
            addRef();

        return *this;
    }

    SmartRef& operator=(const SmartRef& rhs)
    {
        m_ptr = rhs.m_ptr;
        m_refCount = rhs.m_refCount;
        addRef();
    }
    
    SmartRef& operator=(SmartRef&& rhs)
    {
        m_ptr = rhs.m_ptr;
        m_refCount = rhs.m_refCount;
        rhs.m_ptr = nullptr;
        rhs.m_refCount = 0;
    }

    T* operator->() { return m_ptr; }
    const T* operator->() const { return m_ptr; }    
    T& operator*() { return *m_ptr; }    
    const T& operator*() const { return *m_ptr; }

private:
    __forceinline void addRef()
    {
        lock_inc(m_refCount);
    }

    __forceinline void release()
    {
        if(!m_ptr)
            return;

        if(lock_dec(m_refCount) == 0)
        {
            if(!HeapFreeGlobal(m_ptr)) // Note: Dangerous! Code according to this then (either a HeapAlloc-ed object, or a new keyword-ed object).
                delete m_ptr;

            m_ptr = nullptr;
        }
    }

private:
    T* m_ptr = nullptr;
    volatile u32 m_refCount = 0;
};
}