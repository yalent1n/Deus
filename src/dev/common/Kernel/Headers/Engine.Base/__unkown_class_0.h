#pragma once
#include "ahash"
#include <initializer_list>

namespace rw
{
    template<typename Type, typename Hash = unifiedhash<Type>>
    class __unkown_class_0;

    template<typename Type>
    struct HashSetBucketNode
    {
        unsigned hashCode;
        Type value;
        HashSetBucketNode* listNext;
    };

    template<typename Type>
    class HashSetBucketIterator
    {
    public:
        typedef HashSetBucketNode<Type>*  iterator;
        typedef HashSetBucketNode<Type>*  pointer;
        
        typedef Type&                        value_reference;
        typedef Type*                        value_pointer;

        HashSetBucketIterator(pointer node = nullptr)
            : m_node(node)
        {}

        bool empty()                  const { return m_node == nullptr; }
        value_pointer get()           const { return &m_node->value; }

        value_reference operator*()   const { return m_node->value; }
        value_pointer operator->()    const { return &m_node->value; }

        HashSetBucketIterator& operator++()       { m_node = m_node->listNext; return *this; }
        HashSetBucketIterator  operator++(int)    { HashSetBucketIterator tmp(*this); m_node = m_node->listNext; return tmp; }
    private:
        pointer m_node;
    };

    template<typename Type>
    class HashSetIterator
    {
    public:
        typedef HashSetBucketNode<Type>*   pointer;
        typedef HashSetBucketIterator<Type> iterator;

        typedef Type&                               value_reference;
        typedef Type*                               value_pointer;

        HashSetIterator(pointer* buckets = nullptr, pointer* bucketsend = nullptr)
            : m_buckets(buckets), m_bucketsend(bucketsend), m_current(buckets ? *buckets : nullptr)
        {
            if (buckets && (buckets != bucketsend))
            {
                while (*m_buckets == nullptr)
                    ++m_buckets;
                m_current = *m_buckets;
            }
        }

    private:
        void increment_bucket() 
        {
            if(m_buckets)
            {
                ++m_buckets;
                while (*m_buckets == nullptr && (m_buckets != m_bucketsend))
                    ++m_buckets;
                m_current = *m_buckets;
            }
        }

        void increment()
        {
            if(m_buckets)
            {
                m_current++;
                if (m_current.empty())
                {
                    increment_bucket();
                }
            }
        }
    public:
        value_reference operator*()          const { return *m_current; }
        value_pointer operator->()           const { return m_current.get(); }
        HashSetIterator& operator++()            { increment(); return *this; }		
        HashSetIterator  operator++(int)         { HashSetIterator tmp(*this); increment(); return tmp; }

        bool operator==(const HashSetIterator& rhs) const
        {
            return m_buckets == rhs.m_buckets;			
        }
        bool operator!=(const HashSetIterator& rhs) const
        {
            return m_buckets != rhs.m_buckets;			
        }
    protected:
        pointer* m_buckets;
        pointer* m_bucketsend;
        iterator m_current;
    };

    template<typename Type, typename Hash>
    class __unkown_class_0
    {
        static inline const float LOAD_FACTOR					= 0.75f;
		static inline const unsigned DEFAULT_BUCKET_SIZE		= 16;

    public:
        typedef Type                                 value_type;
        typedef HashSetBucketNode<Type>      bucket_type;
        typedef HashSetIterator<Type>           iterator;

        __unkown_class_0();
        __unkown_class_0(const std::initializer_list<Type>& list);
        ~__unkown_class_0();

        float load_size() const
        {
            return (float)m_usedSize / (float)m_tableSize;
        }

        unsigned size() const
        {
            if(!m_table)
			{
				return 0;
			}

			unsigned sz = 0;
			for(iterator it = begin(), e = end(); it != e; it++)
			{
				sz++;
			}
			return sz;
        }

        iterator begin()			{ return iterator(m_table, m_table + m_tableSize); }
		const iterator begin()		const { return iterator(m_table, m_table + m_tableSize); }
		const iterator cbegin()	const { return iterator(m_table, m_table + m_tableSize); }

		iterator end()				{ return iterator(m_table + m_tableSize, m_table + m_tableSize); }
		const iterator end()		const { return iterator(m_table + m_tableSize, m_table + m_tableSize); }
		const iterator cend()		const { return iterator(m_table + m_tableSize, m_table + m_tableSize); }

        int get_bucket_index(unsigned hashCode) const
        {
            return hashCode % m_tableSize;
        }
        int get_bucket_index(const value_type& value) const
        {
            return Hash()(value) % m_tableSize;
        }

        void clear();

        void insert(const value_type& value)
        {
            bucket_type*	bucket;
			unsigned		hash;
			int				bucketIndex;

			increase_storage();

			hash = Hash()(value);
			bucketIndex = hash % m_tableSize;
			bucket = m_table[bucketIndex];

			while (bucket)
			{
				if (bucket->hashCode == hash)
				{
					bucket->value = value;                    
					return;
				}

				bucket = bucket->listNext;
			}

			bucket = m_table[bucketIndex];			

			if (!bucket)
				++m_usedSize;

			bucket_type* pNewNode = new bucket_type();
			pNewNode->hashCode = hash;
			pNewNode->listNext = bucket;
			pNewNode->value = value;
			m_table[bucketIndex] = pNewNode;
        }

        iterator find(const value_type& value) const  
        {
            unsigned hash;
			bucket_type* bucket;
			int bucketIndex;

			// have to do this, because on static hash maps end()
			// will return something like 0x00000080, and that
			// gives a memory read error to whoever calls this function.
			(const_cast<__unkown_class_0*>(this))->increase_storage();

			if (!m_table)
				return end();

			hash			= Hash()(value);
			bucketIndex		= hash % m_tableSize;
			bucket			= m_table[bucketIndex];

			if (!bucket)
				return end();			

			while (bucket)
			{
				if (bucket->hashCode == hash)
				{
					return iterator(&bucket, &bucket);
				}

				bucket = bucket->listNext;
			}

			return end();
        }

        iterator find_by_hash(unsigned hash) const
        {
            bucket_type* bucket;
			int bucketIndex;

			// have to do this, because on static hash maps end()
			// will return something like 0x00000080, and that
			// gives a memory read error to whoever calls this function.
			(const_cast<__unkown_class_0*>(this))->increase_storage();
			
			if (!m_table)
				return end();

			bucketIndex		= hash % m_tableSize;
			bucket			= m_table[bucketIndex];

			if (!bucket)
				return end();			

			while (bucket)
			{
				if (bucket->hashCode == hash)
				{
					return iterator(&bucket, &bucket);
				}

				bucket = bucket->listNext;
			}

			return end();
        }

        void erase(const value_type& value)
        {
            if(!m_table)
				return;
			
			unsigned hash			= Hash()(value);
			unsigned bucketIndex	= hash % m_tableSize;
			bucket_type* node		= m_table[bucketIndex];
			
			if (!node)
				return;

			bucket_type* nodeIt = node;
			
			while (nodeIt)
			{
				bucket_type* next = nodeIt->listNext;
				
				// root node
				if (!next)
				{
					node = nodeIt;					
					if (node->hashCode != hash) // this is the right way, trust this
					{
						return;
					}

                    delete node;
					m_table[bucketIndex] = nullptr;
					m_usedSize--;
					return;										
				}

				if (next->hashCode == hash)
				{
					node = next;
					nodeIt->listNext = next->listNext;
					break;
				}

				nodeIt = nodeIt->listNext;
			}

            delete node;
			m_table[bucketIndex] = nullptr;
        }

    private:
        void increase_storage();

    private:
        bucket_type** m_table;
        int m_usedSize;
        int m_tableSize;        
    };

    template<typename t, typename h>
    __unkown_class_0<t, h>::__unkown_class_0()
        : m_table(nullptr)
        , m_usedSize(0)
        , m_tableSize(0)
    {}

    template<typename t, typename h>
    __unkown_class_0<t, h>::~__unkown_class_0()
    {
        clear();
    }

    template<typename T, typename H>
    void __unkown_class_0<T, H>::clear()
    {
        if(m_table)
        {
            for(int i = 0; i < m_tableSize; i++)
            {
                bucket_type* bucket = m_table[i];
                while(bucket)
                {
                    bucket_type* next = bucket->listNext;
                    delete bucket;
                    bucket = next;
                }

                m_table[i] = nullptr;
            }

            delete m_table;
            m_table = nullptr;
            m_tableSize = m_usedSize = 0;
        }    
    }

    template<typename T, typename H>
    __unkown_class_0<T, H>::__unkown_class_0(const std::initializer_list<T>& list)
    {
        for(auto it = list.begin(); it != list.end(); it++)
        {
            insert(*it);
        }
    }

    template<typename T>
    inline bool insert_into_hash_table(HashSetBucketNode<T>** table, unsigned hashCode, unsigned tableSize, const T& value)
    {
        auto head = table[hashCode % tableSize];
        auto node = new HashSetBucketNode<T>();
        node->hashCode = hashCode;
        node->value = value;
        node->listNext = head;
        table[hashCode % tableSize] = node;

        // head was used before
        return !!(head);
    }

    template<typename T, typename H> 
	void __unkown_class_0<T, H>::increase_storage()
	{				
		if (!m_table)
		{
			m_tableSize = DEFAULT_BUCKET_SIZE;

            m_table = new bucket_type*[m_tableSize];			
			for (int i = 0; i < m_tableSize; ++i)
				m_table[i] = nullptr;
			
			return;
		}		

		if (load_size() > LOAD_FACTOR)
		{
			unsigned prevTableSize = m_tableSize;
			bucket_type** prevTable = m_table;

			unsigned newUsedSize = 0;
			unsigned newTableSize = m_tableSize * 2;
            bucket_type** newTable = new bucket_type*[newTableSize];
			for(unsigned i = 0; i < newTableSize; i++)
			{
				newTable[i] = nullptr;
			}

			//rehash old table to the new table
			for (unsigned slot = 0; slot < prevTableSize; slot++)
			{
				// no head
				bucket_type* listNode = prevTable[slot];
				if(!listNode)
					continue;

				while(listNode)
				{
					bool isNewHead = insert_into_hash_table<T>(
						newTable, 
						listNode->hashCode, 
						newTableSize, 
						listNode->value
					);
					
					if(isNewHead)
					{
						newUsedSize++;
					}

					listNode = listNode->listNext;
				}
			}

			m_table = newTable;
			m_tableSize = newTableSize;
			m_usedSize = newUsedSize;
		}
	}
}