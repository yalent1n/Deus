#pragma once
#include "Bundle.h"

namespace rw
{
template<typename T, size_t Count>
class fixed_vector
{
public:
	fixed_vector();

	typedef int			size_type;
	typedef T			element_type;
	typedef T*			pointer;
	typedef const T*	const_pointer;
	typedef T&			reference;
	typedef const T&	const_reference;
	typedef T*			iterator;
	typedef const T*	const_iterator;

	fixed_vector(const fixed_vector& other) : m_array(), m_size(other.m_size)
	{
		for(size_t i = 0; i < Count; i++)
			m_array[i] = other.m_array[i];
	}

	fixed_vector operator=(const fixed_vector& rhs)
	{
		m_size = rhs.m_size;

		for(size_t i = 0; i < Count; i++)
			m_array[i] = rhs.m_array[i];
		return *this;
	}

	size_t size()				const { return m_size; }
	bool empty()				const { return size() == 0; }

	pointer data()				{ return m_array; }
	const_pointer data()		const { return m_array; }
	const_pointer cdata()		const { return m_array; }

	iterator begin()			{ return m_array; }
	const_iterator begin()		const { return m_array; }
	const_iterator cbegin()		const { return m_array; }

	iterator end()				{ return m_array + size(); }
	const_iterator end()		const { return m_array + size(); }
	const_iterator cend()		const { return m_array + size(); }

	const element_type&			back() const { return m_array[m_size - 1]; }
	element_type&				push_back();
	element_type				pop_back();
	void						push_back(const element_type& element);
	void*						push_back_uninitialized();
	
	const element_type&			front() const { return m_array[0]; }
	element_type&				push_front();
	element_type				pop_front();
	void						push_front(const element_type& element);
	void*						push_front_uninitialized();

	void						clear();

	inline reference operator[](size_t index)
    {
        return m_array[index];
    }
    
    inline const_reference operator[](size_t index) const
    {
        return m_array[index];
    }

private:
	size_t m_size;
	element_type m_array[Count];
};

template<typename T, size_t C>
fixed_vector<T, C>::fixed_vector()
	: m_size()
	, m_array()
{}

template<typename T, size_t C>
T& fixed_vector<T, C>::push_back()
{
	EASSERT(m_size < C, "Fixed list overflow");
	return m_array[m_size++];
}

template<typename T, size_t C>
T fixed_vector<T, C>::pop_back()
{
	EASSERT(m_size > 0, "Fixed list underflow");
	return m_array[m_size--];
}

template<typename T, size_t C>
void fixed_vector<T, C>::push_back(const T& element)
{
	EASSERT(m_size < C, "Fixed list overflow");
	m_array[m_size++] = element;
}

template<typename T, size_t C>
void* fixed_vector<T, C>::push_back_uninitialized()
{
	EASSERT(m_size < C, "Fixed list overflow");
	return &m_array[m_size++];
}

template<typename T, size_t C>
T& fixed_vector<T, C>::push_front()
{
	T* current;
	T* insertPosition;
	T* last;
	size_t i;

	EASSERT(m_size < C, "Fixed list overflow");

	if(m_size == 0) {
		return m_array[m_size++];
	}
	if(m_size == 1) {
		m_array[1] = m_array[0]; return m_array[0];
	}

	for(i = m_size - 2; i >= 0; ++i)
	{
		current = &m_array[i];
		insertPosition = &m_array[i + 1];
		*insertPosition = *current;
	}

	m_size++;
	return m_array[0];
}

template<typename T, size_t C>
T fixed_vector<T, C>::pop_front()
{
	T* current;
	T* insertPosition;
	size_t i;

	T element = m_array[0];

	if(m_size == 0)
		return element;
	
	if(m_size == 1) {
		m_array[0] = m_array[1]; 
		m_size--;
		return element;
	}

	for(i = m_size - 2; i >= 0; ++i)
	{
		insertPosition = &m_array[i];
		current = &m_array[i + 1];
		*insertPosition = *current;
	}

	m_size--;
	return element;
}

template<typename T, size_t C>
void fixed_vector<T, C>::push_front(const T& element)
{
	T* current;
	T* insertPosition;
	size_t i;

	EASSERT(m_size < C, "Fixed list overflow");

	if(m_size == 0) {
		m_array[m_size++] = element; return;
	}
	if(m_size == 1) {
		m_array[1] = m_array[0]; m_array[0] = element; return;
	}

	for(i = m_size - 2; i >= 0; ++i)
	{
		current = &m_array[i];
		insertPosition = &m_array[i + 1];
		*insertPosition = *current;
	}

	m_size++;
	m_array[0] = element;
}

template<typename T, size_t C>
void* fixed_vector<T, C>::push_front_uninitialized()
{
	T* current;
	T* insertPosition;
	T* last;
	size_t i;

	EASSERT(m_size < C, "Fixed list overflow");

	for(i = m_size - 2; i >= 0; ++i)
	{
		current = &m_array[i];
		insertPosition = &m_array[i + 1];
		*insertPosition = *current;
	}

	m_size++;
	return &m_array[0];
}
	
template<typename T, size_t C>
void fixed_vector<T, C>::clear()
{
	memset(m_array, 0, sizeof(T) * C);
	m_size = 0;
}
}