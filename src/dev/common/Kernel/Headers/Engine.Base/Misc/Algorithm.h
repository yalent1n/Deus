#pragma once
#include "../Types.h"

namespace rw::algorithm
{
template<typename T>
struct less
{
    bool operator()(const T& a, const T& b) const
    {
        return a < b;
    }
};

template<typename T>
struct greater
{
    bool operator()(const T& a, const T& b) const
    {
        return a > b;
    }
};

template<typename Iterator>
struct iterator_traits
{
    using value_type = typename Iterator::value_type;
    using reference = typename Iterator::reference;
    using pointer = typename Iterator::pointer;
};

template<typename FirstType, typename SecondType>
struct tuple
{
    FirstType first;
    SecondType second;
};

template<typename T> struct remove_const { using type = T; };    
template<typename T> struct remove_const<const T> { using type = T; };

template<typename T> struct remove_reference { using type = T; };    
template<typename T> struct remove_reference<T&> { using type = T; };    
template<typename T> struct remove_reference<T&&> { using type = T; };

template<typename T> struct remove_pointer { using type = T; };    
template<typename T> struct remove_pointer<T*> { using type = T; };

template<typename ForwardIterator>
ForwardIterator 
min_element(ForwardIterator first, ForwardIterator last)
{
    if(first != last)
    {
        ForwardIterator currentMin = first;

        while(++first != last)
        {
            if(*first < *currentMin)
                currentMin = first;
        }
        return currentMin;
    }
    return first;
}

template<typename ForwardIterator, typename Compare>
ForwardIterator 
min_element(ForwardIterator first, ForwardIterator last, Compare predicate)
{
    if(first != last)
    {
        ForwardIterator currentMin = first;

        while(++first != last)
        {
            if(predicate(*first, *currentMin))
                currentMin = first;
        }
        return currentMin;
    }
    return first;
}

template<typename ForwardIterator>
ForwardIterator 
max_element(ForwardIterator first, ForwardIterator last)
{
    if(first != last)
    {
        ForwardIterator currentMax = first;

        while(++first != last)
        {
            if(*first > *currentMax)
                currentMax = first;
        }
        return currentMax;
    }
    return first;
}

template<typename ForwardIterator, typename Compare>    
ForwardIterator 
max_element(ForwardIterator first, ForwardIterator last, Compare predicate)
{
    if(first != last)
    {
        ForwardIterator currentMax = first;

        while(++first != last)
        {
            if(predicate(*first, *currentMax))
                currentMax = first;
        }
        return currentMax;
    }
    return first;
}

template<typename InputIterator, typename T>
InputIterator 
linear_search(InputIterator first, InputIterator last, const T& value)
{
    while((first != last) && !(*first == value))
        ++first;
    return first;
}

template<typename InputIterator, typename T>
InputIterator 
find_first_of(InputIterator first, InputIterator last, const T& value)
{
    return linear_search(first, last, value);
}

template<typename InputIterator, typename T, typename Compare>
InputIterator 
linear_search(InputIterator first, InputIterator last, const T& value, Compare predicate)
{
    while((first != last) && !predicate(*first, value))
        ++first;
    return first;
}

template<typename InputIterator, typename T, typename Compare>
InputIterator 
find_first_of(InputIterator first, InputIterator last, const T& value, Compare predicate)
{
    return linear_search(first, last, value, predicate);
}

template<typename InputIterator, typename T>
InputIterator 
linear_search_backward(InputIterator first, InputIterator last, const T& value)
{
    while((last != first) && !(*last == value))
        --last;
    return last;
}

template<typename InputIterator, typename T>
InputIterator 
find_last_of(InputIterator first, InputIterator last, const T& value)
{
    return linear_search_backward(first, last, value);
}

template<typename InputIterator, typename T, typename Compare>
InputIterator 
linear_search_backward(InputIterator first, InputIterator last, const T& value, Compare predicate)
{
    while((last != first) && !predicate(*last, value))
        --last;
    return last;
}

template<typename InputIterator, typename T, typename Compare>
InputIterator 
find_last_of(InputIterator first, InputIterator last, const T& value, Compare predicate)
{
    return linear_search_backwards(first, last, value, predicate);
}

template<typename ForwardIterator, typename T>
ForwardIterator
lower_bound(ForwardIterator first, ForwardIterator last, const T& value)
{
    typedef ssize_t DifferenceType;

    DifferenceType d = DifferenceType(last - first);

    while(d > 0)
    {
        ForwardIterator i = first;
        DifferenceType d2 = d >> 1; // on MSVC this is faster than dividing by 2
        i += d2;

        if(*i < value)
        {
            first = ++i;
            d -= d2 + 1;

            if(!d)
                return first - 1;
        }
        else
        {
            d = d2;
        }            
    }

    return first;
}

template<typename ForwardIterator, typename T, typename Compare>
ForwardIterator
lower_bound(ForwardIterator first, ForwardIterator last, const T& value, Compare compare)
{
    typedef ssize_t DifferenceType;

    DifferenceType d = DifferenceType(last - first);

    while(d > 0)
    {
        ForwardIterator i = first;
        DifferenceType d2 = d >> 1; // on MSVC this is faster than dividing by 2
        i += d2;

        if(compare(*i, value))
        {
            first = ++i;
            d -= d2 + 1;

            if(!d)
                return first - 1;
        }
        else
        {
            d = d2;
        }            
    }

    return first;
}

// ----
// Binary search

// May one search be used on an ascendingly sorted list of elements, otherwise false will be returned.
template<typename ForwardIterator, typename T>
bool binary_search(ForwardIterator first, ForwardIterator last, const T& value)
{
    ForwardIterator i(lower_bound(first, last, value));
    return ((i != last) && (value < *i));
}

// May one search be used on an ascendingly sorted list of elements, otherwise false will be returned.
template<typename ForwardIterator, typename T, typename Compare>
bool binary_search(ForwardIterator first, ForwardIterator last, const T& value, Compare compare)
{
    ForwardIterator i(lower_bound(first, last, value, compare));
    return ((i != last) && compare(*i, value));
}

// May one search be used on an ascendingly sorted list of elements, otherwise 'last' will be returned.
template<typename ForwardIterator, typename T>
ForwardIterator binary_search_i(ForwardIterator first, ForwardIterator last, const T& value)
{
    ForwardIterator i(lower_bound(first, last, value));
    if((i != last) && (value < *i))
        return i;
    return last;
}

// May one search be used on an ascendingly sorted list of elements, otherwise 'last' will be returned.
template<typename ForwardIterator, typename T, typename Compare>
ForwardIterator binary_search_i(ForwardIterator first, ForwardIterator last, const T& value, Compare compare)
{
    ForwardIterator i(lower_bound(first, last, value, compare));
    if((i != last) && compare(*i, value))
        return i;
    return last;
}

template<typename ForwardIterator, typename T>
ForwardIterator
upper_bound(ForwardIterator first, ForwardIterator last, const T& value)
{
    typedef ssize_t DifferenceType;

    DifferenceType len = DifferenceType(last - first);

    while(len > 0)
    {
        ForwardIterator i = first;
        DifferenceType  len2 = len >> 1; // on MSVC this is faster than dividing by 2

        i += len2;

        if(!(value < *i))
        {
            first = ++i;
            len -= len2 + 1;
        }
        else
        {
            len = len2;
        }
    }
    return first;
}

template<typename ForwardIterator, typename T, typename Compare>
ForwardIterator
upper_bound(ForwardIterator first, ForwardIterator last, const T& value, Compare compare)
{
    typedef ssize_t DifferenceType;

    DifferenceType len = DifferenceType(last - first);

    while(len > 0)
    {
        ForwardIterator i = first;
        DifferenceType  len2 = len >> 1; // on MSVC this is faster than dividing by 2

        i += len2;

        if(!compare(value, *i))
        {
            first = ++i;
            len -= len2 + 1;
        }
        else
        {
            len = len2;
        }
    }
    return first;
}

template<typename ForwardIterator, typename T, typename Compare = less<T>>
difference_type
count_of(ForwardIterator first, ForwardIterator last, const T& value, Compare compare = less<T>())
{
    ForwardIterator begin = lower_bound(first, last, value, compare);
    ForwardIterator end = upper_bound(first, last, value, compare);
    difference_type d = difference_type(end - begin);
    return d;
}

template<typename Iterator, typename T>
tuple<Iterator, Iterator>
equal_range(Iterator first, Iterator last, const T& value)
{
    return { lower_bound(first, last, value), upper_bound(first, last, value) };
}

inline constexpr difference_type c_smallRangeThreshold = 8;

template<typename Iterator, typename T>
tuple<Iterator, Iterator>
equal_range_small(Iterator first, Iterator last, const T& value)
{
    if(difference_type(last - first) < c_smallRangeThreshold)
    {
        return linear_search(first, last, value);
    }
    
    return equal_range(first, last, value);
}
}