#pragma once
#include "Algorithm.h"
#include <utility>

namespace rw::algorithm
{
namespace helper
{
template<typename integral_type>
inline integral_type log2(integral_type num)
{
	for(int i = 0; num; ++i)
		num >>= 1;

	return -1;
}
}

template<typename Iterator, typename Compare>
void insertion_sort(Iterator first, Iterator last, Compare compare)
{
	typedef typename algorithm::remove_pointer<typename algorithm::remove_reference<Iterator>::type>::type value_type;

	if(first == last)
		return;

	Iterator i = first;

	for(++i; i != last; ++i)
	{
		value_type insertValue(std::move(*i));
		Iterator insertPosition = i;

		for(Iterator movePosition = i; movePosition != first && compare(insertValue, *(--movePosition)); --insertPosition)
			*insertPosition = std::move(*movePosition);

		*insertPosition = std::move(insertValue);
	}
}

template<typename ValueType>
void swap(ValueType& a, ValueType& b)
{
	ValueType c = a;
	a = b;
	b = c;
}

template<typename Iterator, typename Compare>
void heapify(Iterator first, int n, Iterator cursorElement, Compare compare)
{
    typedef ssize_t DifferenceType;

	DifferenceType i = DifferenceType(cursorElement - first);
	DifferenceType largest = i;
	DifferenceType l = (2*i)+1;
	DifferenceType r = (2*i)+2;
	if (l<n && !compare(first[l], first[largest]))
		largest = l;	
	if (r<n && !compare(first[r], first[largest]))
		largest = r;
	if (largest != i) {
		swap(first[i], first[largest]);
		heapify(first, n, first + largest, compare);
	}
}

template<typename Iterator, typename Compare>
inline void heap_sort(Iterator first, Iterator last, Compare compare)
{
    typedef ssize_t DifferenceType;

	DifferenceType n = DifferenceType(last - first);

	for (DifferenceType i=n/2-1;i>=0;i--)
		heapify(first, n, first + i, compare);

	for (DifferenceType i = n - 1; i > 0; i--) {
		swap(first[0], first[i]);
		heapify(first, i, first, compare);
	}
}
}