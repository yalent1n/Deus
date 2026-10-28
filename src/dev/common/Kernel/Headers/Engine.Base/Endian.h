#pragma once
#include "Types.h"

namespace rw
{
enum Endianness : u8
{
    LittleEndian,
    BigEndian,

    PlatformEndian = LittleEndian
};

inline void endianSwap(void* dest, const void* src, unsigned int size, unsigned int count)
{
    char* dest2 = reinterpret_cast<char*>(dest);
	const char* src2 = reinterpret_cast<const char*>(src);

	for (unsigned int i = 0; i < count; ++i)
	{
		for (unsigned int j = 0; j < size; ++j)
			dest2[j] = src2[size - j - 1];

		dest2 += size;
		src2 += size;
	}
}

template <typename T> __forceinline T endianSwap(const T& value)
{
	T ret;

	auto src = reinterpret_cast<const char*>(&value);
	auto dst = reinterpret_cast<char*>(&ret);

	for (unsigned int i = 0; i < sizeof(value); i++)
		dst[i] = src[sizeof(value) - i - 1];

	return ret;
}

template <typename T> __forceinline T toBigEndian(const T& value)
{
#if PlatformEndian == LittleEndian
	return endianSwap(value);
#else
	return value;
#endif
}

template <typename T> __forceinline T toLittleEndian(const T& value)
{
#if PlatformEndian == LittleEndian
	return value;
#else
	return endianSwap(value);
#endif
}

template<typename T> __forceinline T fromBigEndian(const T& value)
{
	return toBigEndian<T>(value);
}

template<typename T> __forceinline T fromLittleEndian(const T& value)
{
	return toLittleEndian<T>(value);
}
}