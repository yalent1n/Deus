#pragma once
#include "../Types.h"

namespace rw
{
class StringEditor;

inline constexpr size_t npos = size_t(-1);

LIB_LINKAGE size_t strlen(const char*);
LIB_LINKAGE size_t wstrlen(const wchar_t*);

LIB_LINKAGE bool isnil(const char*);
LIB_LINKAGE bool wisnil(const wchar_t*);

LIB_LINKAGE bool isspace(const char);
LIB_LINKAGE bool isalnum(const char);
LIB_LINKAGE bool isalchar(const char);
LIB_LINKAGE bool ishexnum(const char);
LIB_LINKAGE bool issep(const char);
LIB_LINKAGE bool wisspace(const wchar_t);
LIB_LINKAGE bool wisalnum(const wchar_t);
LIB_LINKAGE bool wisalchar(const wchar_t);
LIB_LINKAGE bool wishexnum(const wchar_t);
LIB_LINKAGE bool wissep(const wchar_t);

LIB_LINKAGE bool       islower(const char);
LIB_LINKAGE char       tolower(const char);
LIB_LINKAGE bool       islower(const char*, size_t = -1);
LIB_LINKAGE void       setlower(char*, size_t = -1);

LIB_LINKAGE bool       wislower(const wchar_t);
LIB_LINKAGE wchar_t    wtolower(const wchar_t);
LIB_LINKAGE bool       wislower(const wchar_t*, size_t = -1);
LIB_LINKAGE void       wsetlower(char*, size_t = -1);

LIB_LINKAGE bool       isupper(const char);
LIB_LINKAGE char       toupper(const char);
LIB_LINKAGE bool       isupper(const char*, size_t = -1);
LIB_LINKAGE void       setupper(char*, size_t = -1);

LIB_LINKAGE bool       wisupper(const wchar_t);
LIB_LINKAGE char       wtoupper(const wchar_t);
LIB_LINKAGE bool       wisupper(const wchar_t*, size_t = -1);
LIB_LINKAGE void       wsetupper(wchar_t*, size_t = -1);

LIB_LINKAGE const char* safedup(const char*);
LIB_LINKAGE const char* safecpy(char* dst, const char* src);
LIB_LINKAGE const char* safecat(char* dst, const char* src);

LIB_LINKAGE int        compareCase(const char*, const char*, size_t = -1);
LIB_LINKAGE int        compareIgnoreCase(const char*, const char*, size_t = -1);
LIB_LINKAGE bool       equalsCase(const char*, const char*, size_t = -1);
LIB_LINKAGE bool       equalsIgnoreCase(const char*, const char*, size_t = -1);

LIB_LINKAGE int        compareCaseWide(const wchar_t*, const wchar_t*, size_t = -1);
LIB_LINKAGE int        compareIgnoreCaseWide(const wchar_t*, const wchar_t*, size_t = -1);
LIB_LINKAGE bool       equalsCaseWide(const char*, const char*, size_t = -1);
LIB_LINKAGE bool       equalsIgnoreCaseWide(const char*, const char*, size_t = -1);

LIB_LINKAGE void       fixPath(char*, size_t = -1);
LIB_LINKAGE void       fixPathWide(wchar_t*, size_t = -1);

LIB_LINKAGE size_t     indexOf(const char, const char*);
LIB_LINKAGE size_t     lastIndexOf(const char, const char*);

struct HashStringCase
{
    inline unsigned operator()(const char* k) const
    {
        if(isnil(k))
            return 0;

        return hashQuick(k);
    };
};

struct HashStringNoCase
{
    inline unsigned operator()(const char* k) const
    {
        if(isnil(k))
            return 0;

        return hashQuickLowerCase(k);
    };
};

// Ascii to integer conversion with overflow detection
// (no CRT functions seem to include this so we roll our own).
LIB_LINKAGE bool tointeger(const char* str, u64& result);
LIB_LINKAGE bool tointeger(const char* str, s64& result);
LIB_LINKAGE bool tointeger(const char* str, u32& result);
LIB_LINKAGE bool tointeger(const char* str, s32& result);
LIB_LINKAGE bool tointeger(const char* str, u16& result);
LIB_LINKAGE bool tointeger(const char* str, s16& result);
LIB_LINKAGE bool tointeger(const char* str, u8& result);
LIB_LINKAGE bool tointeger(const char* str, s8& result);

LIB_LINKAGE void tostring(s8, StringEditor&);
LIB_LINKAGE void tostring(u8, StringEditor&);
LIB_LINKAGE void tostring(s16, StringEditor&);
LIB_LINKAGE void tostring(u16, StringEditor&);
LIB_LINKAGE void tostring(s32, StringEditor&);
LIB_LINKAGE void tostring(u32, StringEditor&);
LIB_LINKAGE void tostring(s64, StringEditor&);
LIB_LINKAGE void tostring(u64, StringEditor&);
LIB_LINKAGE void tostring(float, StringEditor&);
LIB_LINKAGE void tostring(double, StringEditor&);
}