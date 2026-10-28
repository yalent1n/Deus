#pragma once
#include "ahash"
#include <initializer_list>
#include <stdint.h>

namespace rw
{
	template<typename _keyty, typename _valuety, typename _hash>
	class Dict;

	template<typename _kty, typename _vty>
	struct Pair
	{
		_kty first {};
		_vty second {};

		Pair() = default;
		~Pair() = default;		
	};

	template<typename _kty, typename _vty>
	class BucketNode
	{
	public:
		typedef Pair<_kty, _vty> pair_type;		

		BucketNode() = default;
		~BucketNode() = default;		
	
		unsigned			hashCode		= 0;
		pair_type			pair			{};
		BucketNode*			listNext		= 0;
	};

	template<typename key_type, typename value_type>
	class DictionaryBucketIterator
	{
	public:
		typedef BucketNode<key_type, value_type>*  iterator;
		typedef BucketNode<key_type, value_type>*  pointer;
		
		typedef Pair<key_type, value_type>&         value_reference;
		typedef Pair<key_type, value_type>*         value_pointer;

		DictionaryBucketIterator(pointer node = nullptr)
			: m_node(node)
		{}

		bool empty()                  const { return m_node == nullptr; }
		value_pointer get()           const { return &m_node->pair; }

		value_reference operator*()   const { return m_node->pair; }
		value_pointer operator->()    const { return &m_node->pair; }

		DictionaryBucketIterator& operator++()       { m_node = m_node->listNext; return *this; }
		DictionaryBucketIterator  operator++(int)    { DictionaryBucketIterator tmp(*this); m_node = m_node->listNext; return tmp; }
			
		pointer m_node;
	};

	template<typename key_type, typename value_type>
	class DictionaryIterator
	{
	public:
		typedef BucketNode<key_type, value_type>*     pointer;
		typedef DictionaryBucketIterator<key_type, value_type>  iterator;

		typedef Pair<key_type, value_type>&            value_reference;
		typedef Pair<key_type, value_type>*            value_pointer;

		DictionaryIterator(pointer* buckets = nullptr, pointer* bucketsend = nullptr)
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
		DictionaryIterator& operator++()            { increment(); return *this; }		
		DictionaryIterator  operator++(int)         { DictionaryIterator tmp(*this); increment(); return tmp; }

		bool operator==(const DictionaryIterator& rhs) const
		{
			return m_buckets == rhs.m_buckets;			
		}
		bool operator!=(const DictionaryIterator& rhs) const
		{
			return m_buckets != rhs.m_buckets;			
		}
	
		pointer* m_buckets;
		pointer* m_bucketsend;
		iterator m_current;
	};

	template<typename _keyty, typename _valuety, typename _hash = unifiedhash<_keyty>>
	class Dict
	{
	public:
		using Hash											= _hash;

		typedef _keyty										key_type;
		typedef _valuety									value_type;
		typedef BucketNode<key_type, value_type>			bucket_type;
		typedef Pair<key_type, value_type>					pair_type;
		typedef DictionaryIterator<key_type, value_type>			iterator;

	private:
		static inline const float LOAD_FACTOR					= 0.75f;
		static inline const unsigned DEFAULT_BUCKET_SIZE		= 16;

		bucket_type** m_table = 0;						//> pointer to bucket nodes
		int m_usedSize = 0;								//> count of used buckets (increases when a node is head of a linked list)
		int m_tableSize = DEFAULT_BUCKET_SIZE;			//> capacity of buckets (default is 16)
		void increase_storage();

	public:
		Dict();
		Dict(const std::initializer_list<const Pair<key_type, value_type>>& list);
		Dict(const Dict& rhs)
		{
			if (!rhs.m_table)
            	return;

			m_table = nullptr;
			m_usedSize = rhs.m_usedSize;
			m_tableSize = rhs.m_tableSize;

			// allocate new table and zero it
			m_table = new bucket_type*[m_tableSize];
			for (int i = 0; i < m_tableSize; ++i)
				m_table[i] = nullptr;

			// clone each bucket list preserving order
			for (int i = 0; i < m_tableSize; ++i)
			{
				bucket_type* src = rhs.m_table[i];
				bucket_type* last = nullptr;

				while (src)
				{
					bucket_type* node = new bucket_type();
					node->hashCode = src->hashCode;
					node->pair = src->pair; // copy pair (key/value)
					node->listNext = nullptr;

					if (last)
						last->listNext = node;
					else
						m_table[i] = node;

					last = node;
					src = src->listNext;
				}
			}
		}
		Dict(Dict&& rhs) noexcept
		{
			if(this != &rhs)
			{
				clear();
				m_table = rhs.m_table; rhs.m_table = 0;
				m_usedSize = rhs.m_usedSize; rhs.m_usedSize = 0;
				m_tableSize = rhs.m_tableSize; rhs.m_tableSize = 0;
			}
		}
		~Dict();

		Dict& operator=(const Dict& rhs)
		{
			if (!rhs.m_table)
            	return *this;

			m_table = nullptr;
			m_usedSize = rhs.m_usedSize;
			m_tableSize = rhs.m_tableSize;

			// allocate new table and zero it
			m_table = new bucket_type*[m_tableSize];
			for (int i = 0; i < m_tableSize; ++i)
				m_table[i] = nullptr;

			// clone each bucket list preserving order
			for (int i = 0; i < m_tableSize; ++i)
			{
				bucket_type* src = rhs.m_table[i];
				bucket_type* last = nullptr;

				while (src)
				{
					bucket_type* node = new bucket_type();
					node->hashCode = src->hashCode;
					node->pair = src->pair; // copy pair (key/value)
					node->listNext = nullptr;

					if (last)
						last->listNext = node;
					else
						m_table[i] = node;

					last = node;
					src = src->listNext;
				}
			}

			return *this;
		}
		Dict& operator=(Dict&& rhs) noexcept
		{
			if(this != &rhs)
			{
				clear();
				m_table = rhs.m_table; rhs.m_table = 0;
				m_usedSize = rhs.m_usedSize; rhs.m_usedSize = 0;
				m_tableSize = rhs.m_tableSize; rhs.m_tableSize = 0;
			}

			return *this;
		}

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

		iterator begin()			{ return m_table ? iterator(m_table, m_table + m_tableSize) : iterator(); }
		const iterator begin()		const { return m_table ? iterator(m_table, m_table + m_tableSize) : iterator(); }

		iterator end()				{ return m_table ? iterator(m_table + m_tableSize, m_table + m_tableSize) : iterator(); }
		const iterator end()		const { return m_table ? iterator(m_table + m_tableSize, m_table + m_tableSize) : iterator(); }
		
		int get_bucket_index(unsigned hashCode)	const { return hashCode % m_tableSize; }
		int get_bucket_index(const key_type& key) const { return Hash()(key) % m_tableSize; }

		void clear();

		pair_type& insert(const key_type& key, const value_type& value)
		{
			bucket_type*	bucket;
			unsigned		hash;
			int				bucketIndex;

			increase_storage();

			hash = Hash()(key);
			bucketIndex = hash % m_tableSize;
			bucket = m_table[bucketIndex];

			while (bucket)
			{
				if (bucket->hashCode == hash)
				{
					bucket->pair.second = value;						
					return bucket->pair;
				}

				bucket = bucket->listNext;
			}

			bucket = m_table[bucketIndex];			

			if (!bucket)
				++m_usedSize;

			bucket_type* pNewNode = new bucket_type();
			pNewNode->hashCode = hash;
			pNewNode->listNext = bucket;
			pNewNode->pair = { key, value };
			m_table[bucketIndex] = pNewNode;

			return pNewNode->pair;
		}

		pair_type& insert(const key_type& key)
		{
			bucket_type*	bucket;
			unsigned		hash;
			int				bucketIndex;

			increase_storage();

			hash = Hash()(key);
			bucketIndex = hash % m_tableSize;
			bucket = m_table[bucketIndex];
			
			if (!bucket)
				++m_usedSize;

			while (bucket)
			{
				if (bucket->hashCode == hash)
				{
					return bucket->pair;
				}

				bucket = bucket->listNext;
			}			

			bucket_type* pNewNode = new bucket_type();
			pNewNode->hashCode = hash;
			pNewNode->listNext = bucket;
			pNewNode->pair.first = key;
			//new (&pNewNode->pair.second) value_type();
			m_table[bucketIndex] = pNewNode;

			return pNewNode->pair;
		}

		iterator find(const key_type& key) const
		{
			unsigned hash;
			bucket_type* bucket;
			int bucketIndex;

			// have to do this, because on static hash maps end()
			// will return something like 0x00000080, and that
			// gives a memory read error to whoever calls this function.
			(const_cast<Dict*>(this))->increase_storage();

			if (!m_table)
				return end();

			hash			= Hash()(key);
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
			(const_cast<Dict*>(this))->increase_storage();
			
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

		void erase(const key_type& key)
		{
			if(!m_table)
				return;
			
			unsigned hash			= Hash()(key);
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
		
		void erase(const iterator iterator)
		{
			if(!m_table)
				return;
			
			DictionaryBucketIterator<key_type, value_type> bucket = iterator.m_current;
			if(bucket.empty())
				return;

			bucket_type* node = bucket.m_node;
			if(!node)
				return;

			// FIXME: DANGLING NODE IN THE TABLE, FIND OUT HOW TO GET BUCKET HEAD.
			delete node;
			//m_table[bucketIndex] = nullptr;
		}

		inline value_type& operator[](const key_type& key)
		{
			iterator pair = find(key);

			if (pair == end())
				return insert(key).second;

			return pair->second;
		}
	};

	template<typename _keyty, typename _valuety, typename _hash>
	Dict<_keyty, _valuety, _hash>::Dict( )
	{
		//increase_storage();
	}
	
	template<typename _keyty, typename _valuety, typename _hash>
	Dict<_keyty, _valuety, _hash>::Dict( const std::initializer_list<const Pair<key_type, value_type>>& list )
	{
		for(auto it = list.begin(); it != list.end(); it++)
		{
			insert( it->first, it->second );
		}
	}

	template<typename _keyty, typename _valuety, typename _hash>
	Dict<_keyty, _valuety, _hash>::~Dict()
	{
		clear();
	}

	template<typename _keyty, typename _valuety, typename _hash>
	void Dict<_keyty, _valuety, _hash>::clear()
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

			delete[] m_table;
			m_table = nullptr;
			m_tableSize = m_usedSize = 0;
		}
	}

	template<typename kty, typename vty>
	inline bool insert_into_hash_table(BucketNode<kty, vty>** table, unsigned hashCode, unsigned tableSize, const kty& key, const vty& value)
	{
		BucketNode<kty, vty>* head = table[hashCode % tableSize];

		BucketNode<kty, vty>* node = new BucketNode<kty, vty>();
		node->hashCode = hashCode;
		node->pair.first = key;
		node->pair.second = value;
		node->listNext = head;
		table[hashCode % tableSize] = node;

		// head was used before
		return !!(head);
	}

	template<typename kty, typename vty, typename hf> 
	void Dict<kty, vty, hf>::increase_storage()
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
					bool isNewHead = insert_into_hash_table<kty, vty>(
						newTable, 
						listNode->hashCode, 
						newTableSize, 
						listNode->pair.first, 
						listNode->pair.second
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

namespace eastl
{
	template<typename _keyty, typename _valuety, typename _hash = rw::unifiedhash<_keyty>>
	using hash_map = rw::Dict<_keyty, _valuety, _hash>;
}