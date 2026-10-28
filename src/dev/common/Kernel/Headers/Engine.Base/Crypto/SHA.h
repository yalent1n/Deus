#pragma once
#include "../Bundle.h"

namespace rw
{
template<size_t _BitSize>
struct ShaImpl
{
	inline static constexpr size_t _ByteSize = _BitSize / 8;

	u8 hash[_ByteSize] { 0 };

	FB_BASE_API ShaImpl() = default;
	FB_BASE_API ShaImpl(u8*);

	FB_BASE_API void calculate(const void* _Ptr, size_t _Size);
	FB_BASE_API void calculateHmac(const void* _Ptr, size_t _Size, const void* _Key, size_t _KeySize);
	FB_BASE_API bool parse(const char* _String);
	FB_BASE_API bool parse(const char* _String, size_t _Length);

	FB_BASE_API bool operator==(const ShaImpl&) const;
	FB_BASE_API bool operator!=(const ShaImpl&) const;		
	FB_BASE_API int operator<(const ShaImpl&) const;
};

using SHA1 = ShaImpl<160>;
using SHA256 = ShaImpl<256>;
using SHA384 = ShaImpl<384>;
using SHA512 = ShaImpl<512>;

extern template struct ShaImpl<160>;
extern template struct ShaImpl<256>;
extern template struct ShaImpl<384>;
extern template struct ShaImpl<512>;

//static inline const SHA1 c_emptySha {};

template<class _TImp>
struct ShaStr
{
	inline static constexpr size_t BUFFER_LENGTH = (_TImp::_ByteSize * 2) + 1;
	alignas(8) char str[BUFFER_LENGTH] {0};

	ShaStr() = default;
	FB_BASE_API ShaStr(const _TImp& imp);
};

extern template struct ShaStr<SHA1>;
extern template struct ShaStr<SHA256>;
extern template struct ShaStr<SHA384>;
extern template struct ShaStr<SHA512>;

using Sha1Str = ShaStr<SHA1>;
using Sha256Str = ShaStr<SHA256>;
using Sha384Str = ShaStr<SHA384>;
using Sha512Str = ShaStr<SHA512>;
}