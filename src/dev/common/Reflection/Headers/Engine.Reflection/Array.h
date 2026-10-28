#pragma once
#include "ArrayBase.h"

namespace rw
{
template<typename T>
class Array : public ArrayBase
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
    typedef u32             size_type;
    typedef u32             difference_type;

public:
    Array();
    Array(const std::initializer_list<value_type>&);
    Array(const Array& other);
    Array(Array&& other);
    ~Array();

    Array& operator=(const Array& rhs);
    Array& operator=(Array&& rhs);

    bool empty() const;

    void clear();
    pointer detach();

    pointer reserve(size_type);
    pointer resize(size_type);
        
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
    pointer push_back_uninitialized();
    reference push_back(const_reference);
    reference pop_back();
    reference back();
    const_reference back() const;

    inline reference operator[](size_t index)
    {
        return mBegin[index];
    }
    
    inline const_reference operator[](size_t index) const
    {
        return mBegin[index];
    }

    template<typename... Args>
    inline reference emplace(iterator iter, Args... args)
    {
        return *(new (iter) value_type(args...));
    }

    template<typename... Args>
    inline reference emplace_back(Args... args)
    {
        return *(new (push_back_uninitialized()) value_type(args...));
    }
    

private:
    void ensure(size_type);

private:
    T* mBegin;
};

template<typename T>
class ArrayReader
{
public:
    ArrayReader(const Array<T>& array)
        : m_array(const_cast<Array<T>*>(&array))
    {}

    typename Array<T>::const_iterator begin() const       { return m_array->begin(); }
    typename Array<T>::const_iterator end() const         { return m_array->end(); }

    typename Array<T>::size_type size() const             { return m_array->size(); }
    typename Array<T>::size_type capacity() const         { return m_array->capacity(); }

    template<typename Predicate>
    typename Array<T>::iterator find_if(Predicate pred)
    {
        return m_array->find_if(pred);
    }
    
    template<typename Predicate>
    typename Array<T>::const_iterator find_if(Predicate pred) const
    {
        return m_array->find_if(pred);
    }
    
    template<typename Compare>
    typename Array<T>::iterator find(typename Array<T>::const_reference v, Compare compare)
    {
        return m_array->find(v, compare);
    }
    
    template<typename Compare>
    typename Array<T>::const_iterator find(typename Array<T>::const_reference v, Compare compare) const
    {
        return m_array->find(v, compare);
    }

    typename Array<T>::reference operator[](size_t index)
    {
        return m_array->operator[](index);
    }
    
    typename Array<T>::const_reference operator[](size_t index) const
    {
        return m_array->operator[](index);
    }

private:
    Array<T>* m_array;
};

template<typename T>
class ArrayEditor
{
public:
    ArrayEditor(const Array<T>& array)
        : m_array(const_cast<Array<T>*>(&array))
    {}

    typename Array<T>::const_iterator begin() const       { return m_array->begin(); }
    typename Array<T>::iterator begin()                   { return m_array->begin(); }
    typename Array<T>::const_iterator end() const         { return m_array->end(); }
    typename Array<T>::iterator end()                     { return m_array->end(); }

    typename Array<T>::size_type size() const             { return m_array->size(); }
    typename Array<T>::size_type capacity() const         { return m_array->capacity(); }

    typename Array<T>::reference append()
    { 
        return m_array->push_back();
    }
    
    typename Array<T>::reference append(const T& val)
    { 
        return m_array->push_back(val); 
    }

    template<typename... Args>
    typename Array<T>::reference emplace(typename Array<T>::iterator iter, Args... args)
    {
        return *(new (iter) value_type(args...));
    }

    template<typename... Args>
    typename Array<T>::reference emplace_back(Args... args)
    {
        return *(new (m_array->push_back_uninitialized()) value_type(args...));
    }

    template<typename Predicate>
    typename Array<T>::iterator find_if(Predicate pred)
    {
        return m_array->find_if(pred);
    }
    
    template<typename Predicate>
    typename Array<T>::const_iterator find_if(Predicate pred) const
    {
        return m_array->find_if(pred);
    }
    
    template<typename Compare>
    typename Array<T>::iterator find(typename Array<T>::const_reference v, Compare compare)
    {
        return m_array->find(v, compare);
    }
    
    template<typename Compare>
    typename Array<T>::const_iterator find(typename Array<T>::const_reference v, Compare compare) const
    {
        return m_array->find(v, compare);
    }

    typename Array<T>::iterator insert_range(typename Array<T>::iterator iter, typename Array<T>::const_iterator start, typename Array<T>::const_iterator end)
    {
        return m_array->insert_range(iter, start, end);
    }

    typename Array<T>::iterator insert(typename Array<T>::iterator iter, typename Array<T>::const_reference v)
    {
        return m_array->insert(iter, v);
    }

    typename Array<T>::iterator erase_range(typename Array<T>::const_iterator start, typename Array<T>::const_iterator end)
    {
        return m_array->erase_range(start, end);
    }

    typename Array<T>::iterator erase(typename Array<T>::const_iterator iter)
    {
        return m_array->erase(iter);
    }

    typename Array<T>::reference operator[](size_t index)
    {
        return m_array->operator[](index);
    }
    
    typename Array<T>::const_reference operator[](size_t index) const
    {
        return m_array->operator[](index);
    }
private:
    Array<T>* m_array;
};

template<typename T>
Array<T>::Array()
    : mBegin((T*)&array_helper::s_emptyArrayData[1])
{}

template<typename T>
Array<T>::Array(const std::initializer_list<T>& l)
    : mBegin((T*)array_helper::s_emptyArrayData)
{
    ensure(l.size());

    for(const T& v : l)
        push_back(v);    
}

template<typename T>
Array<T>::Array(const Array& l)
    : mBegin(l.mBegin)
{}

template<typename T>
Array<T>::~Array()
{
    clear();    
}

template<typename T>
bool Array<T>::empty() const
{
    return size() == 0;
}

template<typename T>
void Array<T>::clear()
{
    if(mBegin && mBegin != (T*)&array_helper::s_emptyArrayData[1])
    {
        for(size_type i = 0; i < size(); i++)
        {
            mBegin[i].~T();
        }

        HeapFreeGlobal((u32*)mBegin - array_helper::kEntryCount);
        mBegin = (T*)&array_helper::s_emptyArrayData[1];
    }
}

template<typename T>
typename Array<T>::pointer Array<T>::detach()
{
    pointer array = mBegin;
    mBegin = &array_helper::s_emptyArrayData[1];
    return array;
}

template<typename T>
typename Array<T>::pointer Array<T>::reserve(size_type _AbsoluteCount)
{
    ensure(_AbsoluteCount - size());
    return mBegin;
}

template<typename T>
typename Array<T>::pointer Array<T>::resize(size_type _AbsoluteCount)
{
    size_type nPrevSize = size();

    if (_AbsoluteCount == nPrevSize)
        return mBegin;

    if (_AbsoluteCount == 0)
    {
        Array<T>::~Array();
        return nullptr;
    }

    if (_AbsoluteCount < nPrevSize)
    {
        erase_range(&mBegin[_AbsoluteCount], end());
        return mBegin;
    }

    // 4 is the default capacity
    _AbsoluteCount = _AbsoluteCount > 4 ? _AbsoluteCount : 4;
    T* new_array_ptr = (T*) HeapAllocGlobal((_AbsoluteCount * sizeof(T)) + (array_helper::kEntryCount * sizeof(u32)), alignof(T));
    new_array_ptr = (T*)((u32*)new_array_ptr + array_helper::kEntryCount);

    if(nPrevSize) {
        std::memcpy((void*)new_array_ptr, (const void*)mBegin, nPrevSize * sizeof(T));
    }

    HeapFreeGlobal((u32*)mBegin + array_helper::kEntrySize);
    mBegin = new_array_ptr;
    set_capacity(_AbsoluteCount);
    set_size(nPrevSize);
    return mBegin;
}

template<typename T>
void Array<T>::set_size(size_type _AbsoluteCount)
{
    ((u32*)mBegin)[array_helper::kEntrySize] = _AbsoluteCount;
}

template<typename T>
void Array<T>::set_capacity(size_type _Capacity)
{
    ((u32*)mBegin)[array_helper::kEntryCapacity] = _Capacity;
}

template<typename T>
const typename Array<T>::size_type Array<T>::size() const
{
    if(mBegin && mBegin != (T*)&array_helper::s_emptyArrayData[1])
    {
        return ((u32*)mBegin)[array_helper::kEntrySize];
    }

    return 0;
}

template<typename T>
const typename Array<T>::size_type Array<T>::capacity() const
{
    if(mBegin && mBegin != (T*)&array_helper::s_emptyArrayData[1])
    {
        return ((u32*)mBegin)[array_helper::kEntryCapacity];
    }

    return 0;
}

template<typename T>
void Array<T>::ensure(size_type _Count)
{
    if(!mBegin || mBegin == (T*)&array_helper::s_emptyArrayData[1])
    {
        // In this case it's an initialization, so _Count is the "default capacity".
        mBegin = (T*) HeapAllocGlobal((_Count * sizeof(T)) + (sizeof(u32) * array_helper::kEntryCount), alignof(T));
        mBegin = (T*)((u32*)mBegin + array_helper::kEntryCount);
        set_capacity(_Count);
        return;
    }    

    size_type prevsize = size();
    size_type maxcnt = prevsize + _Count;    
    if(maxcnt <= capacity())
        return;

    size_type alloccnt = (maxcnt + 7) & ~7;

    T* array = (T*) HeapAllocGlobal((alloccnt * sizeof(T)) + (sizeof(u32) * array_helper::kEntryCount), alignof(T));
    array = (T*)((u32*)array + array_helper::kEntryCount);

    if(!std::memcpy((void*)array, (const void*)mBegin, prevsize * sizeof(T)))
        throw;

    HeapFreeGlobal((u32*)mBegin - array_helper::kEntryCount);
    mBegin = array;

    set_size(prevsize);
    set_capacity(alloccnt);
}

template<typename T>
typename Array<T>::pointer Array<T>::begin()
{
    return mBegin == (T*)&array_helper::s_emptyArrayData[1] ? nullptr : mBegin;
}

template<typename T>
typename Array<T>::const_pointer Array<T>::begin() const
{
    return mBegin == (T*)&array_helper::s_emptyArrayData[1] ? nullptr : mBegin;
}

template<typename T>
typename Array<T>::pointer Array<T>::end()
{
    return mBegin + size();
}

template<typename T>
typename Array<T>::const_pointer Array<T>::end() const
{
    return mBegin + size();
}

template<typename T>
template<typename Predicate>
typename Array<T>::iterator Array<T>::find_if(Predicate pred)
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
typename Array<T>::const_iterator Array<T>::find_if(Predicate pred) const
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
typename Array<T>::iterator Array<T>::find(const_reference v, Compare compare)
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
typename Array<T>::const_iterator Array<T>::find(const_reference v, Compare compare) const
{
    for(const_iterator i = begin(), e = end(); i < e; ++i)
    {
        if(compare(v, *i))
            return i;
    }

    return end();
}

template<typename T>
typename Array<T>::iterator Array<T>::insert_range(iterator iter, const_iterator range_start, const_iterator range_end)
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
        set_size(range_count);
        set_capacity(range_count);
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

    set_size(size() + range_count);

    ssize_t index_end = size() - 1;
    for (ssize_t i = index_end; i >= index && i >= 0; i--) {
        mBegin[i + 1] = mBegin[i]; // shift elements to the right
    }

    std::memcpy( array_start, range_start, range_count * sizeof(T) );
    return iter + range_count - 1;
}

template<typename T>
typename Array<T>::iterator Array<T>::insert(iterator iter, const_reference v)
{
    return insert_range(iter, (const_iterator)&v, (const_iterator)(&v + 1));
}

template<typename T>
typename Array<T>::iterator Array<T>::erase_range(const_iterator rangeStart, const_iterator rangeEnd)
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

    std::memset((void*)const_cast<iterator>(rangeStart), 0, rangeSize * sizeof(T));
    std::memcpy((void*)const_cast<iterator>(rangeStart), (const void*)rangeEnd, (size_type)(arrayEnd - rangeEnd) * sizeof(T));
    set_size(size() - rangeSize);
    return mBegin + currentEnd;
}

template<typename T>
typename Array<T>::iterator Array<T>::erase(const_iterator iter)
{
    return erase_range(iter, iter + 1);
}

template<typename T>
typename Array<T>::reference Array<T>::push_front()
{
    return front();
}

template<typename T>
typename Array<T>::reference Array<T>::push_front(typename Array<T>::const_reference v)
{
    return front();
}

template<typename T>
typename Array<T>::reference Array<T>::pop_front()
{
    return *(++mBegin);
}

template<typename T>
typename Array<T>::reference Array<T>::front()
{
    return mBegin[0];
}

template<typename T>
typename Array<T>::const_reference Array<T>::front() const
{
    return mBegin[0];
}

template<typename T>
typename Array<T>::reference Array<T>::push_back()
{
    ensure(1);
    u32& cnt = *((u32*)mBegin + array_helper::kEntrySize);
    reference back = mBegin[cnt++];
    new (&back) T();
    return back;
}

template<typename T>
typename Array<T>::pointer Array<T>::push_back_uninitialized()
{
    ensure(1);
    u32& cnt = *((u32*)mBegin + array_helper::kEntrySize);
    reference back = mBegin[cnt++];
    return &back;
}

template<typename T>
typename Array<T>::reference Array<T>::push_back(typename Array<T>::const_reference v)
{
    ensure(1);
    u32& cnt = *((u32*)mBegin + array_helper::kEntrySize);
    reference back = mBegin[cnt++];
    new (&back) T(v);
    return back;
}

template<typename T>
typename Array<T>::reference Array<T>::pop_back()
{
    u32& cnt = *((u32*)mBegin + array_helper::kEntrySize);
    return mBegin[--cnt];
}

template<typename T>
typename Array<T>::reference Array<T>::back()
{
    if(mBegin && mBegin != (T*)&array_helper::s_emptyArrayData[1])
    {
        return mBegin[*((u32*)mBegin + array_helper::kEntrySize) - 1];
    }

    return *(typename Array<T>::pointer)(nullptr);
}

template<typename T>
typename Array<T>::const_reference Array<T>::back() const
{
    if(mBegin && mBegin != (T*)&array_helper::s_emptyArrayData[1])
    {
        return mBegin[*((u32*)mBegin + array_helper::kEntrySize) - 1];
    }

    return *(typename Array<T>::pointer)(nullptr);
}
}