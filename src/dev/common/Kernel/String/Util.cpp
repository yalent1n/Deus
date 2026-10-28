#include <Engine.Base/String/Util.h>
#include <Engine.Base/String/String.h>
#include <Engine.Base/Memory/VirtualAlloc.h>
#include <emmintrin.h>
#include <wchar.h>
#include <stdint.h>
#include <stdio.h>

namespace rw
{
size_t strlen(const char* s)
{
    if(!s)
        return 0;

    const char* start = s;
    __m128i zero = _mm_setzero_si128();

    for (;;) {
        // Load 16 bytes from the string.
        __m128i data = _mm_loadu_si128((const __m128i*)s);

        // Compare each u8 to zero.
        __m128i result = _mm_cmpeq_epi8(data, zero);

        // Get a mask of all the highest bits. If any byte was 0,
        // its corresponding bit in the mask will be set.
        rw::size_t mask = _mm_movemask_epi8(result);

        // If a null byte was found, break the loop.
        if (mask != 0) {
            // Use __builtin_ctz to find the index of the first null byte.
            // This is a fast, single-instruction way to find the index of the lowest set bit.
            unsigned long index = __builtin_ctz(mask);
            return (s - start) + index;
        }
        // No null found, advance by 16 bytes.
        s += 16;
    }
}

size_t wstrlen(const wchar_t* s)
{
    if(!s)
        return 0;

    const wchar_t* start = s;    
    __m128i zero = _mm_setzero_si128();

    for (;;) {
        __m128i data = _mm_loadu_si128((const __m128i*)s);
        __m128i result = _mm_cmpeq_epi16(data, zero);
        rw::size_t mask = _mm_movemask_epi8(result);

        if (mask != 0) {            
            unsigned long bitindex = _tzcnt_u32(mask);
            return (s - start) + (bitindex / 2);
        }
        // No null found, advance by 16 bytes.
        s += 8;
    }
}

bool isnil(const char* s)
{
    return !s || *s == '\0';
}

bool wisnil(const wchar_t* s)
{
    return !s || *s == L'\0';
}

//ANSI
bool isspace(const char c)
{
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v' || c == '\0');
}

bool isalnum(const char c)
{
    return (c >= '0' && c <= '9');
}

bool isalchar(const char c)
{
    return (islower(c) || isupper(c));
}

bool ishexalnum(const char c)
{
    return isalnum(c) || (islower(c) || isupper(c));
}

bool issep(const char c)
{
    return c == '/' || c == '\\' || c == '\0';
}

//WIDE
bool wisspace(const wchar_t c)
{
    return (c == L' ' || c == L'\t' || c == L'\r' || c == L'\n' || c == L'\f' || c == L'\v' || c == L'\0');
}

bool wisalnum(const wchar_t c)
{
    return (c >= L'0' && c <= L'9');
}

bool wisalchar(const wchar_t c)
{
    return (wislower(c) || wisupper(c));
}

bool wishexalnum(const wchar_t c)
{
    return isalnum(c) || (wislower(c) || wisupper(c));
}

bool wissep(const wchar_t c)
{
    return c == L'/' || c == L'\\' || c == L'\0';
}

//ANSI
bool islower(const char c)
{
    return (c >= 'a' && c <= 'z');
}

char tolower(const char c)
{
    return (isupper(c) ? c + 32 : c);
}

bool islower(const char* s, size_t len)
{
    if(s)
    {
        if(len == -1)
            len = strlen(s);        

        for(size_t i = 0; i < len; i++)
        {
            if(isupper(s[i]))
                return false;            
        }

        return true;
    }

    return false;
}

void setlower(char* s, size_t len)
{
    if(s)
    {
        if(len == -1)
            len = strlen(s);

        for(size_t i = 0; i < len; i++)
            s[i] = tolower(s[i]);
    }
}

//WIDE
bool wislower(const wchar_t c)
{
    return (c >= L'a' && c <= L'z');
}

wchar_t wtolower(const wchar_t c)
{
    return (wisupper(c) ? c + 32 : c);
}

bool wislower(const wchar_t* s, size_t len)
{
    if(s)
    {
        if(len == -1)
            len = wstrlen(s);

        for(size_t i = 0; i < len; i++)
        {
            if(wisupper(s[i]))
                return false;            
        }

        return true;
    }

    return false;
}

void wsetlower(wchar_t* s, size_t len)
{
    if(s)
    {
        if(len == -1)
            len = wstrlen(s);

        for(size_t i = 0; i < len; i++)
            s[i] = wtolower(s[i]);
    }
}

//ANSI
bool isupper(const char c)
{
    return c >= 'A' && c <= 'Z';
}

char toupper(const char c)
{
    return (islower(c) ? c - 32 : c);
}

bool isupper(const char* s, size_t len)
{
    if(s)
    {
        if(len == -1)
            len = strlen(s);        

        for(size_t i = 0; i < len; i++)
        {
            if(islower(s[i]))
                return false;            
        }

        return true;
    }

    return false;
}

void setupper(char* s, size_t len)
{
    if(s)
    {
        if(len == -1)
            len = strlen(s);

        for(size_t i = 0; i < len; i++)
            s[i] = toupper(s[i]);
    }
}

//WIDE
bool wisupper(const wchar_t c)
{
    return c >= L'A' && c <= L'Z';
}

char wtoupper(const wchar_t c)
{
    return (wislower(c) ? c - 32 : c);
}

bool wisupper(const wchar_t* s, size_t len)
{
    if(s)
    {
        if(len == -1)
            len = wstrlen(s);        

        for(size_t i = 0; i < len; i++)
        {
            if(wislower(s[i]))
                return false;            
        }

        return true;
    }

    return false;
}

void wsetupper(wchar_t* s, size_t len)
{
    if(s)
    {
        if(len == -1)
            len = wstrlen(s);

        for(size_t i = 0; i < len; i++)
            s[i] = wtoupper(s[i]);
    }
}

const char* safedup(const char* s)
{
    if(isnil(s))
        return nullptr;

    size_t len = std::strlen(s);
    char* ns = (char*) HeapAllocGlobal(len + 1);
    std::memcpy(ns, s, len);
    ns[len] = '\0';
    return ns;
}

const char* safecpy(char* dst, const char* src)
{
    if(!dst || !src)
        return nullptr;

    size_t len = strlen(src);
    std::memcpy(dst, src, len);
    return dst;
}

const char* safecat(char* dst, const char* src)
{
    if(!dst || !src)
        return nullptr;

    size_t srcLen = strlen(src);
    if(!srcLen)
        return dst;

    char* dest = dst + strlen(dst);
    std::memcpy(dest, src, srcLen);
    return dst;
}

int compareCase(const char* s1, const char* s2, size_t maxlen)
{
    if(!s1 && !s2)
        return 0;

    if(maxlen == -1) {
        size_t a = strlen(s1), b = strlen(s2);        
        if(a != b)
            return int(a - b);

        maxlen = a;
    }

    for(size_t i = 0; i < maxlen; i++) {
        const char a = s1[i];
        const char b = s2[i];
        if(a != b)
            return int(a - b);
    }

    return 0;
}

int compareIgnoreCase(const char* s1, const char* s2, size_t maxlen)
{
    if(!s1 && !s2)
        return 0;

    if(maxlen == -1) {
        size_t a = strlen(s1), b = strlen(s2);        
        if(a != b)
            return int(a - b);

        maxlen = a;
    }

    for(size_t i = 0; i < maxlen; i++) {
        const char a = tolower(s1[i]);
        const char b = tolower(s2[i]);
        if(a != b)
            return int(a - b);
    }

    return 0;
}

bool equalsCase(const char* s1, const char* s2, size_t maxlen)
{
    return compareCase(s1, s2, maxlen) == 0;
}

bool equalsIgnoreCase(const char* s1, const char* s2, size_t maxlen)
{
    return compareIgnoreCase(s1, s2, maxlen) == 0;
}

int compareCaseWide(const wchar_t* s1, const wchar_t* s2, size_t maxlen)
{
    if(!s1 && !s2)
        return 0;

    if(maxlen == -1) {
        size_t a = wstrlen(s1), b = wstrlen(s2);
        if(a != b)
            return int(a - b);

        maxlen = a;
    }

    for(size_t i = 0; i < maxlen; i++) {
        const wchar_t a = s1[i];
        const wchar_t b = s2[i];
        if(a != b)
            return int(a - b);
    }

    return 0;
}

int compareIgnoreCaseWide(const wchar_t* s1, const wchar_t* s2, size_t maxlen)
{
    if(!s1 && !s2)
        return 0;

    if(maxlen == -1) {
        size_t a = wstrlen(s1), b = wstrlen(s2);
        maxlen = (a < b ? a : b);
    }

    for(size_t i = 0; i < maxlen; i++) {
        const wchar_t a = wtolower(s1[i]);
        const wchar_t b = wtolower(s2[i]);
        if(a != b)
            return int(a - b);
    }

    return 0;
}

bool equalsCaseWide(const wchar_t* s1, const wchar_t* s2, size_t maxlen)
{
    return compareCaseWide(s1, s2, maxlen) == 0;
}

bool equalsIgnoreCaseWide(const wchar_t* s1, const wchar_t* s2, size_t maxlen)
{
    return compareIgnoreCaseWide(s1, s2, maxlen) == 0;
}

void fixPath(char* p, size_t len)
{
    if(p && len)
    {
        if(len == -1)
            len = strlen(p);

        for(size_t i = 0; i < len; i++)
        {
#ifdef SvcCompilerMSVC
            if(p[i] == '/' || !p[i])
                p[i] = '\\';
#else
            if(p[i] == '\\' || !p[i])
                p[i] = '/';
#endif
        }
    }
}

void fixPathWide(wchar_t* p, size_t len)
{
    if(p && len)
    {
        if(len == -1)
            len = wstrlen(p);

        for(size_t i = 0; i < len; i++)
        {
#ifdef SvcCompilerMSVC
            if(p[i] == L'/')
                p[i] = L'\\';
#else
            if(p[i] == L'\\')
                p[i] = L'/';
#endif
        }
    }
}

size_t indexOf(const char c, const char* s)
{
    if(isnil(s))
        return size_t(-1);

    const char* sc = s;
    for(;;)
    {
        if(!*sc) {
            if(!c) {
                return size_t(sc - s);
            }

            return size_t(-1);
        }

        if(*sc == c)
            break;

        ++sc;
    }

    return size_t(sc - s);
}

size_t lastIndexOf(const char c, const char* s)
{
    ssize_t idx = 0;

	// go to the end of string
	while (s[idx++] != 0);

	// check in reverse
	while((s[idx--] != c) && (idx >= -1))
        ;

    if(idx > 0)
    {
        return size_t(idx + 1);
    }

	return npos;
}

__forceinline int chartodigit(char c)
{
	return ((c) >= '0' && (c) <= '9' ? (c) - '0' : -1);
}

template <typename integer>
bool willOverflow(integer total, integer c, integer max)
{
	if (total < (max / 10))
		return false;
	else if (total > (max / 10))
		return true;
	else if (c > (max - (max / 10) * 10))
		return true;
	else
		return false;
}

template <typename integer>
bool willOverflow(integer total, integer c, bool isNegative, integer min, integer max)
{
	if (isNegative)
	{
		total = -total;
		c = -c;
		if (total > (min / 10))
			return false;
		else if (total < (min / 10))
			return true;
		else if (c < (min - (min / 10) * 10))
			return true;
		else
			return false;
	}
	else
		return willOverflow<integer>(total, c, max);
}

bool 
tointeger(const char* ptr, /* out */ u64& result)
{
    if(isnil(ptr))
        return false;

	const char* nptr = ptr;
	int c;              /* current char */
	u64 total;			/* current total */
	int sign;           /* if '-', then negative, otherwise positive */

	while ( isspace(*nptr) )
		++nptr;

	c = (int)*nptr++;
	sign = c;						// save sign indication

	if (c == '-' || c == '+')
		c = (int)*nptr++;			// skip sign

	total = 0;

	while ( (c = chartodigit(char(c)) ) != -1 )
	{
		if (willOverflow<u64>(total, c, UINT64_MAX))
			return false;
		
		total	= 10 * total + c;   // accumulate digit

		c		= *nptr++;			// get next char
	}

	result = total;

	if ((ptr + 1) == nptr)
		return false;				// We didn't accept any characters

	if (sign == '-')
		return false;

	return true;
}

bool 
tointeger(const char* ptr, /* out */ s64& result)
{
    if(isnil(ptr))
        return false;

	const char* nptr = ptr;

	while ( isspace((int)*nptr) )
		++nptr;

	int c = (int)*nptr++;
	
	int sign;						// if '-', then negative, otherwise positive
	sign = c;						// save sign indication

	if (c == '-' || c == '+')
		c = (int)*nptr++;			// skip sign

	s64 total = 0;					// current total

	while ( (c = chartodigit(char(c)) ) != -1 )
	{
		if (willOverflow<s64>(total, c, sign == '-', INT64_MIN, INT64_MAX))
			return false;
		
		total	= 10 * total + c;   // accumulate digit

		c		= *nptr++;			// get next char
	}

	if ((ptr + 1) == nptr)
		return false;				// We didn't accept any characters

	if (sign == '-')
		result = -total;
	else
		result = total;

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool 
tointeger(const char* ptr, /* out */ u32& result)
{
    if(isnil(ptr))
        return false;

	const char* nptr = ptr;
	int c;              /* current char */
	u32 total;			/* current total */
	int sign;           /* if '-', then negative, otherwise positive */

	while ( isspace((int)*nptr) )
		++nptr;

	c = (int)*nptr++;
	sign = c;						// save sign indication

	if (c == '-' || c == '+')
		c = (int)*nptr++;			// skip sign

	total = 0;

	while ( (c = chartodigit(char(c)) ) != -1 ) 
	{
		if (willOverflow<u32>(total, c, UINT32_MAX))
			return false;
		
		total	= 10u * total + c;  // accumulate digit

		c		= *nptr++;			// get next char
	}

	result = total;

	if ((ptr + 1) == nptr)
		return false;				// We didn't accept any characters

	if (sign == '-')
		return false;

	return true;
}

bool 
tointeger(const char* ptr, /* out */ s32& result)
{
    if(isnil(ptr))
        return false;

	const char* nptr = ptr;

	while ( isspace((int)*nptr) )
		++nptr;

	int c = (int)*nptr++;

	int sign;						// if '-', then negative, otherwise positive
	sign = c;						// save sign indication

	if (c == '-' || c == '+')
		c = (int)*nptr++;			// skip sign

	s32 total = 0;					// current total

	while ( (c = chartodigit(char(c)) ) != -1 ) 
	{
		if (willOverflow<s32>(total, c, sign == '-', INT32_MIN, INT32_MAX))
			return false;
		
		total	= 10 * total + c;   // accumulate digit

		c		= *nptr++;			// get next char
	}

	if ((ptr + 1) == nptr)
		return false;				// We didn't accept any characters

	if (sign == '-')
		result = -total;
	else
		result = total;

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool tointeger(const char* str, /* out */ u16& result)
{
    if(isnil(str))
        return false;

	u32 t = result;

	bool success = tointeger(str, t);

	if (t > UINT16_MAX)
		return false;

	result = u16(t);

	return success;
}

bool tointeger(const char* str, /* out */ s16& result)
{
    if(isnil(str))
        return false;

	s32 t = result;

	bool success = tointeger(str, t);

	if (t > INT16_MAX || t < INT16_MIN)
		return false;

	result = s16(t);

	return success;
}

bool tointeger(const char* str, /* out */ u8& result)
{
    if(isnil(str))
        return false;

	u32 t = result;

	bool success = tointeger(str, t);

	if (t > UINT8_MAX)
		return false;

	result = u8(t);

	return success;
}

bool tointeger(const char* str, /* out */ s8& result)
{
    if(isnil(str))
        return false;

	s32 t = result;

	bool success = tointeger(str, t);

	if (t > INT8_MAX || t < INT8_MIN)
		return false;

	result = s8(t);

	return success;
}

void tostring(s8 n, StringEditor& result)
{
    int numLen = snprintf(NIL, 0, "%d", n);
    char* num = (char*) _malloca(numLen + 1);
    snprintf(num, numLen + 1, "%d", n);
    num[numLen] = '\0';

    result.appendChars(num, u32(numLen));
}

void tostring(u8 n, StringEditor& result)
{
    int numLen = snprintf(NIL, 0, "%u", n);
    char* num = (char*) _malloca(numLen + 1);
    snprintf(num, numLen + 1, "%u", n);
    num[numLen] = '\0';

    result.appendChars(num, u32(numLen));
}

void tostring(s16 n, StringEditor& result)
{
    int numLen = snprintf(NIL, 0, "%d", n);
    char* num = (char*) _malloca(numLen + 1);
    snprintf(num, numLen + 1, "%d", n);
    num[numLen] = '\0';

    result.appendChars(num, u32(numLen));
}

void tostring(u16 n, StringEditor& result)
{
    int numLen = snprintf(NIL, 0, "%u", n);
    char* num = (char*) _malloca(numLen + 1);
    snprintf(num, numLen + 1, "%u", n);
    num[numLen] = '\0';

    result.appendChars(num, u32(numLen));
}

void tostring(s32 n, StringEditor& result)
{
    int numLen = snprintf(NIL, 0, "%d", n);
    char* num = (char*) _malloca(numLen + 1);
    snprintf(num, numLen + 1, "%d", n);
    num[numLen] = '\0';

    result.appendChars(num, u32(numLen));
}

void tostring(u32 n, StringEditor& result)
{
    int numLen = snprintf(NIL, 0, "%u", n);
    char* num = (char*) _malloca(numLen + 1);
    snprintf(num, numLen + 1, "%u", n);
    num[numLen] = '\0';

    result.appendChars(num, u32(numLen));
}

void tostring(s64 n, StringEditor& result)
{
    int numLen = snprintf(NIL, 0, "%lld", n);
    char* num = (char*) _malloca(numLen + 1);
    snprintf(num, numLen + 1, "%lld", n);
    num[numLen] = '\0';

    result.appendChars(num, u32(numLen));
}

void tostring(u64 n, StringEditor& result)
{
    int numLen = snprintf(NIL, 0, "%llu", n);
    char* num = (char*) _malloca(numLen + 1);
    snprintf(num, numLen + 1, "%llu", n);
    num[numLen] = '\0';

    result.appendChars(num, u32(numLen));
}

void tostring(float n, StringEditor& result)
{
    int numLen = snprintf(NIL, 0, "%.2f", n);
    char* num = (char*) _malloca(numLen + 1);
    snprintf(num, numLen + 1, "%.2f", n);
    num[numLen] = '\0';

    result.appendChars(num, u32(numLen));
}

void tostring(double n, StringEditor& result)
{
    int numLen = snprintf(NIL, 0, "%.2f", n);
    char* num = (char*) _malloca(numLen + 1);
    snprintf(num, numLen + 1, "%.2f", n);
    num[numLen] = '\0';

    result.appendChars(num, u32(numLen));
}
}