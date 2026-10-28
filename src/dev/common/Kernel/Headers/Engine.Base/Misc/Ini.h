#pragma once
#include "../Bundle.h"
#include <functional>
#include "../Vec.h"

namespace rw
{
struct datIniElement
{
    const char* Key;
    const char* Value;
};

typedef eastl::vector<datIniElement> datIniElements;

struct datIniSection
{
    char* Name;
    const datIniElement* Elements;
    u32 ElementCount;
};

typedef eastl::vector<datIniSection> datIniSections;

class datIni
{
public:
    typedef std::function<void(const datIniSection*)> SectionCallback;
    
    static FB_BASE_API datIni* Load(const char* Path);
    static FB_BASE_API bool ParseChars(char* chars, char* charsEnd, datIniSections& Sections);

public:
    datIni() = default;
    ~datIni() = default;

    FB_BASE_API const datIniSection* GetSection(const char* Name) const;
    FB_BASE_API datIniSection* GetSection(const char* Name);

    FB_BASE_API void EnumerateSections(SectionCallback);

private:
    eastl::vector<datIniSection> m_Sections;
};
}