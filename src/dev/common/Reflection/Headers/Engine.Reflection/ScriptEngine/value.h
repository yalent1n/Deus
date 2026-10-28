#pragma once
#include "command.h"

namespace rw
{
typedef size_t scrAny;

enum scrValueType
{
    VT_NULL,

    VT_BOOL,
    VT_BYTE,
    VT_BYTE_UNSIGNED,
    VT_SHORT,
    VT_SHORT_UNSIGNED,
    VT_INT,
    VT_INT_UNSIGNED,
    VT_LONG,
    VT_LONG_UNSIGNED,
    VT_FLOAT,
    VT_DOUBLE,
    VT_PROGRAM,
    VT_PROC, // native
    VT_REFERENCE,
    VT_STRING
};

struct scrValue
{   
    union {
        s8 Byte;
        u8 UByte;
        s16 Short;
        u16 UShort;
        s32 Int;
        u32 UInt;
        s64 Long;
        u64 ULong;
        float Float;
        double Double;
        scrInsn* Program;
        scrCommand Command;
        scrValue* Reference;
        const char* String;
        size_t Any;
    };
};

struct scrValueEq
{
    inline bool operator()(const scrValue& a, const scrValue& b) const
    {
        return a.Any == b.Any;
    }
};

struct scrValueLess
{
    inline bool operator()(const scrValue& a, const scrValue& b) const
    {
        return a.Any < b.Any;
    }
};

struct scrValueGreater
{
    inline bool operator()(const scrValue& a, const scrValue& b) const
    {
        return a.Any > b.Any;
    }
};
}