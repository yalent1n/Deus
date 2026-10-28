#pragma once
#include "Bundle.h"
#include <Engine.Base/Memory/VirtualAlloc.h>
#include <initializer_list>
#include <cstring>

namespace rw
{
template<typename T>
class Vec
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

    Vec(bool MakeDefaultSpace = false);
    Vec(const std::initializer_list<value_type>&);
    Vec(const Vec& other)
        : mBegin(other.mBegin)
        , mSize(other.mSize)
        , mCapacity(other.mCapacity)
    {}
    Vec(Vec&& other)
        : mBegin(other.mBegin)
        , mSize(other.mSize)
        , mCapacity(other.mCapacity)
    {
        other.mBegin = nullptr;
        other.mSize = 0;
        other.mCapacity = 0;
    }

    ~Vec();

    bool empty() const;

    void clear();
    pointer detach();

    pointer reserve(size_type);
    pointer resize(size_type);
        
    void set_data(iterator);
    void set_size(size_type);
    void set_capacity(size_type);

    const size_type size() const;
    const size_type capacity() const;

    pointer begin();
    const_pointer begin() const;
    
    pointer end();
    const_pointer end() const;
    
    template<typename Predicate>
    iterator find_if(Predicate);
    
    template<typename Predicate>
    const_iterator find_if(Predicate) const;
    
    template<typename Compare>
    iterator find(const_reference, Compare);
    
    template<typename Compare>
    const_iterator find(const_reference, Compare) const;

    iterator insert_range(iterator iter, const_iterator start, const_iterator end);
    iterator insert(iterator iter, const_reference);

    iterator erase_range(const_iterator start, const_iterator end);
    iterator erase(const_iterator iter);

    reference push_front();
    reference push_front(const_reference);
    reference pop_front();
    reference front();
    const_reference front() const;
    
    reference push_back();
    reference push_back(const_reference);
    reference pop_back();
    reference back();
    const_reference back() const;

    inline Vec& operator=(const Vec& rhs)
    {
        mBegin = rhs.mBegin;
        mSize = rhs.mSize;
        mCapacity = rhs.mCapacity;
        return *this;
    }

    inline Vec& operator=(Vec&& rhs)
    {
        mBegin = rhs.mBegin;
        rhs.mBegin = nullptr;
        mSize = rhs.mSize;
        rhs.mSize = 0;
        mCapacity = rhs.mCapacity;
        rhs.mCapacity = 0;
        return *this;
    }

    inline reference operator[](size_t index)
    {
        return mBegin[index];
    }
    
    inline const_reference operator[](size_t index) const
    {
        return mBegin[index];
    }

    void ensure(size_type);

private:
    pointer    mBegin;
    size_type       mSize;
    size_type       mCapacity;
};

template<typename T>
Vec<T>::Vec(bool MakeDefaultSpace)
    : mBegin(nullptr)
    , mSize(0)
    , mCapacity(0)
{
    constexpr size_type kDefaultCapacity = 4;
    if(MakeDefaultSpace)
    {
        ensure(kDefaultCapacity);
    }
}

template<typename T>
Vec<T>::Vec(const std::initializer_list<T>& l)
    : mBegin(nullptr)
    , mSize(0)
    , mCapacity(0)
{
    ensure(l.size());

    for(const T& v : l)
        push_back(v);    
}
template<typename T>
Vec<T>::~Vec()
{
    clear();    
}

template<typename T>
bool Vec<T>::empty() const
{
    return !(mSize);
}

template<typename T>
void Vec<T>::clear()
{
    if(mBegin)
    {
        for(size_type i = 0; i < mSize; i++)
        {
            mBegin[i].~T();
        }

        HeapFreeGlobal(mBegin);
        mBegin = nullptr;
    }

    mSize = mCapacity = 0;
}

template<typename T>
typename Vec<T>::pointer Vec<T>::detach()
{
    pointer array = mBegin;
    mBegin = nullptr;
    return array;
}

template<typename T>
typename Vec<T>::pointer Vec<T>::reserve(size_type _AbsoluteCount)
{
    ensure(_AbsoluteCount - mSize);
    return mBegin;
}

template<typename T>
typename Vec<T>::pointer Vec<T>::resize(size_type _AbsoluteCount)
{
    size_type nPrevSize = size();

    if (_AbsoluteCount == nPrevSize)
        return mBegin;

    if (_AbsoluteCount == 0)
    {
        Vec<T>::~Vec();
        return nullptr;
    }

    if (_AbsoluteCount < nPrevSize)
    {
        erase_range(&mBegin[_AbsoluteCount], end());
        return mBegin;
    }

    // 4 is the default capacity
    _AbsoluteCount = _AbsoluteCount > 4 ? _AbsoluteCount : 4;
    T* new_array_ptr = (T*) HeapAllocGlobal(_AbsoluteCount * sizeof(T));
    if(nPrevSize) {
        std::memcpy(new_array_ptr, mBegin, nPrevSize * sizeof(T));
    }
    HeapFreeGlobal(mBegin);
    mBegin = (T*)new_array_ptr;
    mCapacity = _AbsoluteCount;
    mSize = nPrevSize;
    return mBegin;
}

template<typename T>
void Vec<T>::set_data(iterator iter)
{
    mBegin = iter;
}

template<typename T>
void Vec<T>::set_size(size_type _AbsoluteCount)
{
    mSize = _AbsoluteCount;
    if(!mCapacity)
        mCapacity = _AbsoluteCount;
}

template<typename T>
void Vec<T>::set_capacity(size_type _Capacity)
{
    mCapacity = _Capacity;
}

template<typename T>
const typename Vec<T>::size_type Vec<T>::size() const
{
    return mSize;
}

template<typename T>
const typename Vec<T>::size_type Vec<T>::capacity() const
{
    return mCapacity;
}

template<typename T>
void Vec<T>::ensure(size_type _Count)
{
    if(_Count <= 0)
        return;

    if(!mBegin)
    {
        // In this case it's an initialization, so _Count is the "default capacity".
        mBegin = (T*) HeapAllocGlobal(_Count * sizeof(T));
        mCapacity = _Count;
        mSize = 0;
        return;
    }    

    size_type maxcnt = mSize + _Count;    
    if(maxcnt <= mCapacity)
        return;

    size_type alloccnt = (maxcnt + 7) & ~7;

    T* array = (T*) HeapAllocGlobal(alloccnt * sizeof(T));
    if(!std::memcpy((void*)array, (const void*)mBegin, maxcnt * sizeof(T)))
        throw;

    HeapFreeGlobal(mBegin);
    mBegin = array;

    mCapacity = alloccnt;
}

template<typename T>
typename Vec<T>::pointer Vec<T>::begin()
{
    return mBegin;
}

template<typename T>
typename Vec<T>::const_pointer Vec<T>::begin() const
{
    return mBegin;
}

template<typename T>
typename Vec<T>::pointer Vec<T>::end()
{
    return mBegin + mSize;
}

template<typename T>
typename Vec<T>::const_pointer Vec<T>::end() const
{
    return mBegin + mSize;
}

template<typename T>
template<typename Predicate>
typename Vec<T>::iterator Vec<T>::find_if(Predicate pred)
{
    for(iterator i = begin(), e = end(); i < e; ++i)
    {
        if(pred(*i))
            return i;
    }

    return end();
}

template<typename T>
template<typename Predicate>
typename Vec<T>::const_iterator Vec<T>::find_if(Predicate pred) const
{
    for(const_iterator i = begin(), e = end(); i < e; ++i)
    {
        if(pred(*i))
            return i;
    }

    return end();
}

template<typename T>
template<typename Compare>
typename Vec<T>::iterator Vec<T>::find(const_reference v, Compare compare)
{
    for(iterator i = begin(), e = end(); i < e; ++i)
    {
        if(compare(v, *i))
            return i;
    }

    return end();
}

template<typename T>
template<typename Compare>
typename Vec<T>::const_iterator Vec<T>::find(const_reference v, Compare compare) const
{
    for(const_iterator i = begin(), e = end(); i < e; ++i)
    {
        if(compare(v, *i))
            return i;
    }

    return end();
}

template<typename T>
typename Vec<T>::iterator Vec<T>::insert_range(iterator iter, const_iterator range_start, const_iterator range_end)
{
    iterator			array_start;
    iterator			array_end;
    iterator			insert_end;
    size_type				range_count;
    
    array_start				= iter;
    array_end				= mBegin + capacity();
    range_count				= size_type(range_end - range_start);
    insert_end			    = array_start + range_count;

    if(!mBegin)
    {
        resize(range_count);
        std::memcpy( mBegin, range_start, range_count * sizeof(T) );
        mSize = range_count;
        mCapacity = range_count;
        return mBegin;
    }

    EASSERT(iter <= end(), "Inserting after array end is undefined!");
    ssize_t index = ssize_t(iter - mBegin);

    // if the range end is out of bounds
    if(array_end < insert_end)
    {
        const size_type current_count = size_type(end() - array_start);
        const size_type remainder_count = size_type(range_count - current_count);
        resize( remainder_count );
        array_start = mBegin + index; // needed if resize creates a new array
    }

    // EXAMPLE:
    // (34 12, size: 2) -> (00 34 12, size: 3)
    // 
    
    ssize_t index_end = size() - 1;
    for (ssize_t i = index_end; i >= index && i >= 0; i--) {
        mBegin[i + 1] = mBegin[i]; // shift elements to the right
    }
    
    mSize += range_count;
    std::memcpy( array_start, range_start, range_count * sizeof(T) );
    return array_start + range_count - 1;
}

template<typename T>
typename Vec<T>::iterator Vec<T>::insert(iterator iter, const_reference v)
{
    return insert_range(iter, (const_iterator)&v, (const_iterator)(&v + 1));
}

template<typename T>
typename Vec<T>::iterator Vec<T>::erase_range(const_iterator rangeStart, const_iterator rangeEnd)
{
    size_type		rangeSize;
    size_type		currentEnd;
    iterator	arrayEnd;

    if (empty())
    {
        ensure(1);
        return begin();
    }

    rangeSize = (size_type)(rangeEnd - rangeStart);
    currentEnd = (size_type)(rangeEnd - begin());
    arrayEnd = end();

    if (!rangeSize)
        return mBegin + currentEnd;

    for (iterator it = const_cast<iterator>(rangeStart); it < rangeEnd; it++)
        it->~T();

    std::memset(const_cast<iterator>(rangeStart), 0, rangeSize * sizeof(T));
    std::memcpy(const_cast<iterator>(rangeStart), rangeEnd, (size_type)(arrayEnd - rangeEnd) * sizeof(T));
    mSize -= rangeSize;
    return mBegin + currentEnd;
}

template<typename T>
typename Vec<T>::iterator Vec<T>::erase(const_iterator iter)
{
    return erase_range(iter, iter + 1);
}

template<typename T>
typename Vec<T>::reference Vec<T>::push_front()
{
    return front();
}

template<typename T>
typename Vec<T>::reference Vec<T>::push_front(typename Vec<T>::const_reference v)
{
    return front();
}

template<typename T>
typename Vec<T>::reference Vec<T>::pop_front()
{
    if(!mSize)
        return *mBegin;
        
    --mSize;
    return *(mBegin++);
}

template<typename T>
typename Vec<T>::reference Vec<T>::front()
{
    return mBegin[0];
}

template<typename T>
typename Vec<T>::const_reference Vec<T>::front() const
{
    return mBegin[0];
}

template<typename T>
typename Vec<T>::reference Vec<T>::push_back()
{
    ensure(1);
    reference back = mBegin[mSize++];
    new (&back) T();
    return back;
}

template<typename T>
typename Vec<T>::reference Vec<T>::push_back(typename Vec<T>::const_reference v)
{
    ensure(1);
    reference back = mBegin[mSize++];
    new (&back) T(v);
    return back;
}

template<typename T>
typename Vec<T>::reference Vec<T>::pop_back()
{
    return mBegin[--mSize];
}

template<typename T>
typename Vec<T>::reference Vec<T>::back()
{
    return mBegin[mSize - 1];
}

template<typename T>
typename Vec<T>::const_reference Vec<T>::back() const
{
    return mBegin[mSize - 1];
}
}

namespace eastl
{
template<typename T>
using vector = rw::Vec<T>;
}