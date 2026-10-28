#include <Engine.Base/Memory/Util.h>
#include <emmintrin.h>

namespace rw
{
void* memcpy(void* d, const void* s, size_t n, const size_t slen)
{
    if(!s || !d || !slen || !n)
        return nullptr;

    if(slen < n)
        n = slen;
    
    while (n >= sizeof(__m128i)) {
        __m128i t = _mm_loadu_si128((const __m128i*)s);
        _mm_store_si128((__m128i*)d, t);
        d = (__m128i*)d+1;
        s = (const __m128i*)s+1;
        n -= sizeof(__m128i);
    }

    while (n > 0) {
        *(u8*)d = *(const u8*)s;
        d = (u8*)d+1; 
        s = (const u8*)s+1;
        n--;
    }

    return d;
}

void* memset(void* d, u8 b, size_t n)
{
    if (!d || !n)
        return nullptr;
    
    while (n >= sizeof(__m128i)) {
        _mm_store_si128((__m128i*)d, _mm_set1_epi8(b));
        d = (__m128i*)d + 1;
        n -= sizeof(__m128i);
    }
    
    while (n > 0) {
        *(u8*)d=b;
        d = (u8*)d+1;
        n--;
    }

    return d;
}
}