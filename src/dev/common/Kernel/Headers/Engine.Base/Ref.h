#pragma once
#include "Types.h"

namespace rw
{
template<typename T>
class Ref
{
public:
    Ref() = default;
    Ref(const T* p) : m_ptr(const_cast<T*>(p)) {}
    Ref(const Ref& other) : m_ptr(other.m_ptr) {}
    Ref(Ref&& other) : m_ptr(other.m_ptr) { other.m_ptr = nullptr; }
    ~Ref() { m_ptr = nullptr; }

    inline Ref& operator=(const T* ptr)
    {
        m_ptr = ptr;
    }

    inline Ref& operator=(const Ref& rhs)
    {
        m_ptr = rhs.m_ptr;
    }
    
    inline Ref& operator=(Ref&& rhs)
    {
        m_ptr = rhs.m_ptr;
        rhs.m_ptr = nullptr;
    }

    inline T* operator->()
    {
        return m_ptr;
    }
    
    inline const T* operator->() const
    {
        return m_ptr;
    }
    
    inline T& operator*()
    {
        return *m_ptr;
    }
    
    inline const T& operator*() const
    {
        return *m_ptr;
    }

private:
    T* m_ptr = nullptr;
};

template<typename T>
class ScopedRef
{
public:
    ScopedRef() = default;
    ScopedRef(const T* p) : m_ptr(const_cast<T*>(p)) {}
    ScopedRef(const ScopedRef& other) : m_ptr(other.m_ptr) {}
    ScopedRef(ScopedRef&& other) : m_ptr(other.m_ptr) { other.m_ptr = nullptr; }
    ~ScopedRef() { if(m_ptr) { delete m_ptr; m_ptr = nullptr; } }

    inline bool isNull() const { return m_ptr == nullptr; }

    inline T* get() { return m_ptr; }
    inline const T* get() const { return m_ptr; }
    
    inline T* detach() { T* p = m_ptr; /*delete m_ptr;*/ m_ptr = nullptr; return p; }
    inline const T* detach() const { T* p = m_ptr; /*delete m_ptr;*/ m_ptr = nullptr; return p; }

    inline ScopedRef& operator=(T* ptr)
    {
        m_ptr = ptr;
        return *this;
    }

    inline ScopedRef& operator=(const ScopedRef& rhs)
    {
        m_ptr = rhs.m_ptr;
        return *this;
    }
    
    inline ScopedRef& operator=(ScopedRef&& rhs)
    {
        m_ptr = rhs.m_ptr;
        rhs.m_ptr = nullptr;
        return *this;
    }

    inline T* operator->()
    {
        return m_ptr;
    }
    
    
    const T* operator->() const
    {
        return m_ptr;
    }
    
    inline T** operator&()
    {
        return &m_ptr;
    }
    
    
    const T** operator&() const
    {
        return &m_ptr;
    }

    operator T*() const
    {
        return m_ptr;
    }
    
    inline T& operator*()
    {
        return *m_ptr;
    }
    
    inline const T& operator*() const
    {
        return *m_ptr;
    }

private:
    T* m_ptr = nullptr;
};
}