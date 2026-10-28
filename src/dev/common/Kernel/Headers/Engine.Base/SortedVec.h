#pragma once
#include "Vec.h"
#include "Misc/Sort.h"

namespace rw
{
template<typename T, typename Compare = algorithm::less<T>>
class SortedVec
{
public:
    typedef T               value_type;
    typedef const T         const_value_type;
    typedef T*              pointer;
    typedef const T*        const_pointer;
    typedef T*              iterator;
    typedef const T*        const_iterator;
    typedef T&              reference;
    typedef const T&        const_reference;
    typedef ssize_t         size_type;
    typedef ssize_t         difference_type;

    SortedVec(bool MakeDefaultSpace = false);
    SortedVec(const std::initializer_list<value_type>&);
    ~SortedVec();

    bool empty() const;

    void clear();
    pointer detach();

    pointer reserve(size_type);
    pointer resize(size_type);

    const size_type size() const;
    const size_type capacity() const;

    pointer begin();
    const_pointer begin() const;
    
    pointer end();
    const_pointer end() const;
    
    template<typename Compare>
    iterator find_if(const_reference, Compare);
    
    template<typename Compare>
    const_iterator find_if(const_reference, Compare) const;

    iterator insert(const_reference);

    iterator erase(const_reference);
private:
    Vec<T> m_vec;
};

template<typename T, typename C>
SortedVec<T, C>::SortedVec(bool MakeDefaultSpace)
    : m_vec(MakeDefaultSpace)
{}

template<typename T, typename C>
bool SortedVec<T, C>::empty() const
{
    return m_vec.empty();
}

template<typename T, typename C>
void SortedVec<T, C>::clear()
{
    m_vec.clear();
}

template<typename T, typename C>
typename SortedVec<T, C>::pointer SortedVec<T, C>::detach()
{
    return m_vec.detach();
}

template<typename T, typename C>
typename SortedVec<T, C>::pointer SortedVec<T, C>::reserve(size_type n)
{
    return m_vec.reserve(n);
}

template<typename T, typename C>
typename SortedVec<T, C>::pointer SortedVec<T, C>::resize(size_type n)
{
    return m_vec.resize(n);
}

template<typename T, typename C>
const typename SortedVec<T, C>::size_type SortedVec<T, C>::size() const
{
    return m_vec.size();
}

template<typename T, typename C>
const typename SortedVec<T, C>::size_type SortedVec<T, C>::capacity() const
{
    return m_vec.capacity();
}

template<typename T, typename C>
typename SortedVec<T, C>::pointer SortedVec<T, C>::begin()
{
    return m_vec.begin();
}

template<typename T, typename C>
typename SortedVec<T, C>::const_pointer SortedVec<T, C>::begin() const
{
    return m_vec.begin();
}

template<typename T, typename C>
typename SortedVec<T, C>::pointer SortedVec<T, C>::end()
{
    return m_vec.end();
}

template<typename T, typename C>
typename SortedVec<T, C>::const_pointer SortedVec<T, C>::end() const
{
    return m_vec.end();
}

template<typename T, typename C>
template<typename Compare>
typename SortedVec<T, C>::iterator SortedVec<T, C>::find_if(const_reference v, Compare predicate)
{
    return m_vec.find_if(v, predicate);
}

template<typename T, typename C>
template<typename Compare>
typename SortedVec<T, C>::const_iterator SortedVec<T, C>::find_if(const_reference v, Compare predicate) const
{
    return m_vec.find_if(v, predicate);
}
}