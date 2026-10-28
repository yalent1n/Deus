#pragma once
#include "../Types.h"

namespace rw
{
class String
{
public:
    String(const char* chars = NIL)
        : m_chars(chars)
    {}

    ~String()
    {
        m_chars = nullptr;
    }

    const char* c_str() const
        { return m_chars; }

    u32 size() const
        { return strlen(m_chars); }

private:
    const char* m_chars;
};

inline const char* c_str(const String& str)
{
    return str.c_str();
}
}