#include <Engine.Base/Misc/Ini.h>
#include <Engine.Base/Ref.h>
#include <Engine.Base/Buffer/SharedBuffer.h>

namespace rw
{
char* ReadStringUntil(const char* ptr, char term, u32* keylenout = NULL)
{
	if(!ptr)
		return 0;

	u32 len = 0;

	const char* cursor = ptr;
	while(cursor[len] != term)
		len++;

	char* str = (char*) HeapAllocGlobal(len+1);
	std::memcpy(str, ptr, len);
    str[len] = '\0';

	if(keylenout)
		*keylenout = len;

	return str;
}

char* OmitSpace(char* s)
{
    while(isspace(*s))
        ++s;

    return s;
}

bool datIni::ParseChars(char* chars, char* charsEnd, datIniSections& Sections)
{
    char* csr = chars;
    char* CurrentKey = NULL;
    char* CurrentValue = NULL;
    
    datIniSection Section;
    Section.Name = NULL;
    Section.Elements = NULL;
    Section.ElementCount = NULL;

    size_t CharCount = 0;
    datIniElements Elements;

    while(csr < charsEnd)
    {
        if(*csr == '\t' || *csr == '\f' || *csr == '\v')
        {
            ++csr;
            continue;
        }

        if(*csr == '[')
        {
            if(Section.Name)
            {
                if(Elements.size())
                {
                    Section.ElementCount = Elements.size();
                    Section.Elements = Elements.detach();
                }

                Sections.push_back(Section);
                Section.Name = NULL;
                Section.Elements = NULL;
                Section.ElementCount = NULL;
            }

            u32 amtRead = 0;
            Section.Name = ReadStringUntil(++csr, ']', &amtRead);
            csr += amtRead + 1;
            csr = OmitSpace(csr);
            continue;
        }

        if(*csr == '=')
        {
            ++csr;

            if(CharCount)
            {
                char* cKeyTemp = csr - 1 - CharCount;
                CurrentKey = (char*) HeapAllocGlobal(CharCount + 1);
                memcpy(CurrentKey, cKeyTemp, CharCount);
                CurrentKey[CharCount] = 0;
                csr = OmitSpace(csr);
            }

            CharCount = 0;
            continue;
        }

        if(*csr == '\r' || *csr == '\n')
        {
            if(csr[0] == '\n' && csr[-1] == '\r') 
            {
                --csr;
                --CharCount;
            }

            char* CurrentValueEnd = csr;
            if(csr[0] == '\r')
                ++csr;
            ++csr;

            if(CharCount)
            {
                char* cValueTemp = CurrentValueEnd - CharCount;
                CurrentValue = (char*) HeapAllocGlobal(CharCount + 1);
                memcpy(CurrentValue, cValueTemp, CharCount);
    
                CurrentValue[CharCount] = 0;
                Elements.push_back() = {CurrentKey, CurrentValue};

                CurrentKey = NULL;
                CurrentValue = NULL;
            }

            CharCount = 0;
            continue;
        }

        ++csr;
        ++CharCount;
    }

    // if any value left
    if(CharCount)
    {
        --csr;
        --CharCount;
        
        if(csr[0] == '\n' && csr[-1] == '\r') 
        {
            --csr;
            --CharCount;
        }

        char* CurrentValueEnd = csr;
        if(!isspace(*CurrentValueEnd))
        {
            ++CurrentValueEnd;
            ++CharCount;
        }

        if(csr[0] == '\r')
            ++csr;
        ++csr;
        
        char* cValueTemp = CurrentValueEnd - CharCount;
        CurrentValue = (char*) HeapAllocGlobal(CharCount + 1);
        memcpy(CurrentValue, cValueTemp, CharCount);

        CurrentValue[CharCount] = 0;
        Elements.push_back() = {CurrentKey, CurrentValue};

        CurrentKey = NULL;
        CurrentValue = NULL;
    }

    if(Section.Name && Sections.size())
    {
        if(Elements.size())
        {
            Section.ElementCount = Elements.size();
            Section.Elements = Elements.detach();
        }

        Sections.push_back(Section);
        Section.Name = NULL;
        Section.Elements = NULL;
        Section.ElementCount = NULL;
    }

    return YES;
}

datIni* datIni::Load(const char* Path)
{
    MappedSharedBuffer iniBuf(Path, true);
    char* chars = (char*) iniBuf.bytes();

    ScopedRef ini = new datIni;
    if(!ParseChars(chars, chars + iniBuf.size64(), ini->m_Sections))
        return NULL;

    return ini.detach();
}

const datIniSection* datIni::GetSection(const char* Name) const
{
    if(m_Sections.size())
    {
        for(const datIniSection& sec : m_Sections)
        {
            if(equalsIgnoreCase(Name, sec.Name))
                return &sec;
        }
    }

    return nullptr;
}

datIniSection* datIni::GetSection(const char* Name)
{
    if(m_Sections.size())
    {
        for(datIniSection& sec : m_Sections)
        {
            if(equalsIgnoreCase(Name, sec.Name))
                return &sec;
        }
    }

    return nullptr;
}

void datIni::EnumerateSections(SectionCallback Callback)
{
    for(const datIniSection& sec : m_Sections)
    {
        Callback(&sec);
    }
}
}