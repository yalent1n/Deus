#pragma once
#include "ahash"
#include "Vec.h"

namespace rw
{
// Indexed archive (vector pf elements)
template<typename T, typename Hash = unifiedhash<T>>
class HashSet
{
public:
    typedef Vec<T>          array_type;
    
    typedef T               value_type;
    typedef const T         const_value_type;
    typedef T*              pointer;
    typedef const T*        const_pointer;
    typedef T*              iterator;
    typedef const T*        const_iterator;
    typedef T&              reference;
    typedef const T&        const_reference;
    typedef size_t          size_type;
    typedef ssize_t         difference_type;

private:
    struct indexer
    {
        pointer i_value;
        indexer* i_next;
        u32 i_hash;
    };

    typedef indexer* indexer_type;

public:
    HashSet() = default;
    ~HashSet();

    pointer begin();
    const_pointer begin() const;
    
    pointer end();
    const_pointer end() const;

    void clear();
    pointer detach();

    pointer insert(const_reference);
    pointer get(const_reference) const;
    bool empty() const;

private:
    void ensure();

private:
    array_type mArray;
    indexer_type* mIndicers = nullptr;
};

template<typename T, typename H>
HashSet<T, H>::~HashSet()
{
    clear();
}

template<typename T, typename H>
typename HashSet<T, H>::pointer HashSet<T, H>::begin()
{
    return mArray.begin();
}

template<typename T, typename H>
typename HashSet<T, H>::const_pointer HashSet<T, H>::begin() const
{
    return mArray.begin();
}

template<typename T, typename H>
typename HashSet<T, H>::pointer HashSet<T, H>::end()
{   
    return mArray.end();
}

template<typename T, typename H>
typename HashSet<T, H>::const_pointer HashSet<T, H>::end() const
{
    return mArray.end();
}

template<typename T, typename H>
void HashSet<T, H>::clear()
{
    if(!mIndicers)
        return;
        
    // Clear should actually be called from inside this class only.
    indexer_type* bucketCsr = mIndicers;
    for(size_t i = 0; i < kIndexTableCapacity; i++)
    {
        indexer_type indexCsr = bucketCsr[i];
        while(indexCsr)
        {
            indexer_type next = indexCsr->i_next;
            delete indexCsr;
            indexCsr = next;
        }
    }    
}

template<typename T, typename H>
typename HashSet<T, H>::pointer HashSet<T, H>::detach()
{
    return mArray.detach();
}

inline constexpr size_t kIndexTableCapacity = 16;

template<typename T, typename H>
typename HashSet<T, H>::pointer HashSet<T, H>::insert(const_reference _Element)
{
    u32 hashval = H()(_Element);
    u32 indexval = (hashval % kIndexTableCapacity);

    ensure();
    indexer_type* bucketptr = mIndicers + indexval;

    indexer_type index = new indexer();
    index->i_hash = hashval;
    index->i_value = &mArray.push_back();
    index->i_next = *bucketptr;
    *bucketptr = index;

    *index->i_value = _Element;
    return index->i_value;
}

template<typename T, typename H>
typename HashSet<T, H>::pointer HashSet<T, H>::get(const_reference _Element) const
{
    u32 hashval = H()(_Element);
    u32 indexval = (hashval % kIndexTableCapacity);

    if(!mIndicers)
        return nullptr;

    indexer_type indexCsr = mIndicers[indexval];
    if(indexCsr)
    {
        while(indexCsr && indexCsr->i_hash != hashval)
            indexCsr = indexCsr->i_next;
    }

    if(!indexCsr)
        return nullptr;
        
    if(hashval != indexCsr->i_hash)
        return nullptr;

    return indexCsr->i_value;
}

template<typename T, typename H>
bool HashSet<T, H>::empty() const
{
    return mArray.empty();
}

template<typename T, typename H>
void HashSet<T, H>::ensure()
{
    if(mIndicers)
        return;

    mIndicers = new indexer_type[kIndexTableCapacity];
    for(size_t i = 0; i < kIndexTableCapacity; i++)
    {
        mIndicers[i] = NIL;
    }
}
}