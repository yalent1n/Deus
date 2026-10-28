#pragma once
#include "../Bundle.h"
#include "Util.h"
#include <stdio.h>
#include <vadefs.h>

namespace rw
{	
inline const char Endl = '\n';
inline const wchar_t WEndl = L'\n';

template <typename Ch>
class IStringEditor
{		
public:
    typedef Ch char_type;

    FB_BASE_API IStringEditor() = default;            

    //////////////////////////////////////////////
    // Fixed string builder constructors
    FB_BASE_API IStringEditor(Ch* workspaceBuffer, u32 workspaceSize);            
    //////////////////////////////////////////////
    
    //////////////////////////////////////////////
    // Extendable string builder constructors
    FB_BASE_API IStringEditor(u32 bufferSize);            
    //////////////////////////////////////////////

    FB_BASE_API ~IStringEditor();		

    FB_BASE_API Ch* data()					const;
    FB_BASE_API const Ch* c_str()			const;

    // Returns the number of characters in the buffer.
    FB_BASE_API u32	size()					const;
    FB_BASE_API u32	capacity()				const;

    FB_BASE_API bool isDynamic()			const;
    FB_BASE_API bool isEmpty()				const;
    
    FB_BASE_API const Ch front()			const;
    FB_BASE_API const Ch back()				const;

    FB_BASE_API const Ch* detach();		

    FB_BASE_API void allocReserve(u32);

    // TODO: FINISH IT
    FB_BASE_API void reset()
    {			
    }
    
    FB_BASE_API void clear();

    FB_BASE_API IStringEditor<Ch>& appendChar(const Ch c);
    FB_BASE_API IStringEditor<Ch>& appendChars(const Ch* str);
    FB_BASE_API IStringEditor<Ch>& appendChars(const Ch *begin, u32 size);
    FB_BASE_API IStringEditor<Ch>& appendRange(const Ch *begin, const Ch *end);

    FB_BASE_API __forceinline const Ch operator[](int idx) const							
    { 
        return c_str()[idx]; 
    }

protected:
    u32                 mSize;
    u32					mCapacity;
    Ch*					mBuffer;
    bool				mEnd;
    bool                mDynamicFlag;
};

extern template class IStringEditor<char>;
extern template class IStringEditor<wchar_t>;

class StringEditor : public IStringEditor<char>
{
public:
    StringEditor(char* workspace, u32 size)
        : IStringEditor<char>(workspace, size)
    {}
    
    StringEditor(u32 bufferSize)
        : IStringEditor<char>(bufferSize)
    {}

    // __forceinline StringBase& operator<<(const eastl::string& str)
    // { 
    // 	append(str.c_str(), (u32)str.size()); 
    // 	return *this; 
    // }

    // __forceinline StringBase& operator<<(const String& str)
    // { 
    // 	append(str.c_str()); 
    // 	return *this; 
    // }

    __forceinline StringEditor& operator<<(const char* str)
    { 
        (void) appendChars(str); 
        return *this; 
    }

    __forceinline StringEditor& operator<<(const char c) 
    { 
        (void) appendChar(c);
        return *this; 
    }		

    __forceinline StringEditor& operator<<(const StringEditor& str) 
    { 
        (void) appendChars(str.c_str());
        return *this; 
    }

    // __forceinline StringBase& operator<<(bool b)  { char buf[6]{0}; toString(b, buf); append(buf); return *this; }
    // __forceinline StringBase& operator<<(u8 n)  { char buf[4]{0}; toString(n, buf); append(buf); return *this; }
    // __forceinline StringBase& operator<<(i8 n)  { char buf[5]{0}; toString(n, buf); append(buf); return *this; }
    // __forceinline StringBase& operator<<(u16 n) { char buf[6]{0}; toString(n, buf); append(buf); return *this; }
    // __forceinline StringBase& operator<<(i16 n) { char buf[7]{0}; toString(n, buf); append(buf); return *this; }
    // __forceinline StringBase& operator<<(u32 n) { char buf[11]{0}; toString(n, buf); append(buf); return *this; }
    // __forceinline StringBase& operator<<(i32 n) { char buf[12]{0}; toString(n, buf); append(buf); return *this; }
    // __forceinline StringBase& operator<<(u64 n) { char buf[26]{0}; toString(n, buf); append(buf); return *this; }
    // __forceinline StringBase& operator<<(i64 n) { char buf[26]{0}; toString(n, buf); append(buf); return *this; }
    // __forceinline StringBase& operator<<(f32 n) { char buf[26]{0}; toString(n, buf); append(buf); return *this; }
    // __forceinline StringBase& operator<<(f64 n) { char buf[26]{0}; toString(n, buf); append(buf); return *this; }
    // StringBase& operator<<(const Guid& g);
    // StringBase& operator<<(const SHA1& d);
};	
class WideStringEditor : public IStringEditor<wchar_t>
{
public:
    WideStringEditor(wchar_t* workspace, u32 size)
        : IStringEditor<wchar_t>(workspace, size)
    {}

    WideStringEditor(u32 bufferSize)
        : IStringEditor<wchar_t>(bufferSize)
    {}

    // __forceinline WideStringBase& operator<<(const eastl::wstring& str)
    // { 
    // 	append(str.c_str()); 
    // 	return *this; 
    // }

    __forceinline WideStringEditor& operator<<(const wchar_t* str)
    { 
        (void) appendChars(str);
        return *this; 
    }

    __forceinline WideStringEditor& operator<<(const wchar_t c) 
    { 
        (void) appendChar(c); 
        return *this; 
    }		

    __forceinline WideStringEditor& operator<<(const WideStringEditor& str) 
    { 
        (void) appendChars(str.c_str()); 
        return *this; 
    }
};

template <u32 BufferSize, class Base_t>
class FixedStringTempl : public Base_t
{
    alignas(8) typename Base_t::char_type m_fixedWorkspace[BufferSize] { 0 };

public:
    FixedStringTempl()
        : Base_t(m_fixedWorkspace, BufferSize) 
    {}

    FixedStringTempl(const typename Base_t::char_type* constString)
        : Base_t(m_fixedWorkspace, BufferSize)
    {
        size_t stringLength = strlen(constString);
        EASSERT(stringLength <= BufferSize, "Fixed string length does not fit buffer size");
    }
};	

template <u32 BufferSize, class Base_t>
class DynamicStringTempl : public Base_t
{
public:
    DynamicStringTempl() 
        : Base_t(BufferSize) 
    {}
};

template<u32 BufferSize>
class FormattedStringBuilder
{
    char m_workspace[BufferSize] {0};

public:
    FormattedStringBuilder(const char* fmt, ...)
    {
        va_list args;
        __crt_va_start(args, fmt);
        vsnprintf(m_workspace, BufferSize, fmt, args);
        __crt_va_end(args);
    }

    const char* c_str() const { return m_workspace; }
    char* data() const { return m_workspace; }

    const char front() const { return m_workspace[0]; }
    const char* back() const { return m_workspace[BufferSize - 1]; }
};

template<u32 BufferSize>
class FormattedWideStringBuilder
{
    wchar_t m_workspace[BufferSize] {0};

public:
    FormattedWideStringBuilder(const wchar_t* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        vsnprintf(m_workspace, BufferSize, fmt, args);
        va_end(args);
    }

    const wchar_t* c_str() const { return m_workspace; }
    wchar_t* data() const { return m_workspace; }

    const wchar_t front() const { return m_workspace[0]; }
    const wchar_t* back() const { return m_workspace[BufferSize - 1]; }
};

template<u32 Capacity = 128> using FixedStringBuilder = FixedStringTempl<Capacity, StringEditor>;
template<u32 Capacity = 256> using StringBuilder = DynamicStringTempl<Capacity, StringEditor>;

template<u32 Capacity = 128> using FixedWideStringBuilder = FixedStringTempl<Capacity, WideStringEditor>;
template<u32 Capacity = 256> using WideStringBuilder = DynamicStringTempl<Capacity, WideStringEditor>;

using PathStringBuilder = StringBuilder<260>;
using WidePathStringBuilder = WideStringBuilder<260>;
}

FB_BASE_API void tohexstr(const void*, size_t, rw::StringEditor&, bool _Lowercase = true);
FB_BASE_API void wtohexstr(const void*, size_t, rw::WideStringEditor&, bool _Lowercase = true);
FB_BASE_API void fromhexstr(const char*, size_t, void* _OutBuffer, size_t _OutBufferSize);
FB_BASE_API void wfromhexstr(const wchar_t*, size_t, void* _OutBuffer, size_t _OutBufferSize);

static __forceinline const char* c_str(const rw::StringEditor& sb)
{
	return sb.c_str();
}

template<rw::u32 BufferSize>
static __forceinline const char* c_str(const rw::FormattedStringBuilder<BufferSize>& sb)
{
	return sb.c_str();
}

static __forceinline const wchar_t* c_str(const rw::WideStringEditor& sb)
{
	return sb.c_str();
}

static __forceinline bool isnil(const rw::StringEditor& sb)
{
	return sb.isEmpty();
}

static __forceinline bool isnil(const rw::WideStringEditor& sb)
{
	return sb.isEmpty();
}