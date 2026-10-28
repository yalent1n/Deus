#include <Engine.Base/String/String.h>
#include <Engine.Base/Memory/VirtualAlloc.h>
#include <cstring>

static constexpr const char _CHARS_LOWER[] = "0123456789abcdef";
static constexpr const char _CHARS_UPPER[] = "0123456789ABCDEF";
static constexpr const wchar_t _WCHARS_LOWER[] = L"0123456789abcdef";
static constexpr const wchar_t _WCHARS_UPPER[] = L"0123456789ABCDEF";

namespace rw
{
enum : size_t
{ 
	kDynamicFlag			= 1,
	kPtrMask			= size_t(~1),
};

template<typename T> 
static unsigned _HelperStrlen(const T*) { return 0; }

template<> 
unsigned _HelperStrlen<char>(const char* str)
{ 
	return (unsigned) rw::strlen(str);
}

template<> 
unsigned _HelperStrlen<wchar_t>(const wchar_t* str) 
{ 
	return (unsigned) rw::wstrlen(str);	
}

template<typename Ch>
IStringEditor<Ch>::IStringEditor(Ch* workspaceBuffer, u32 workspaceSize)
    : mSize(0)
    , mCapacity(workspaceSize*sizeof(Ch))
    , mBuffer(workspaceBuffer)
    , mEnd(false)
{
    mDynamicFlag = NO;
}

template<typename Ch>
IStringEditor<Ch>::IStringEditor(u32 charCount)
    : mSize(0) 
    , mCapacity(charCount)
    , mEnd(false)
{
    mDynamicFlag = YES;
    
    mBuffer = (Ch*) HeapAllocGlobal( mCapacity * sizeof(Ch) );
    word masked = (word)mBuffer | kDynamicFlag;
    mBuffer = (Ch*) masked;
}

template<typename Ch>
IStringEditor<Ch>::~IStringEditor()
{
    clear();
}

template<typename Ch> Ch* IStringEditor<Ch>::data()					const { return reinterpret_cast<Ch*>(u64(mBuffer) & kPtrMask); }
template<typename Ch> const Ch* IStringEditor<Ch>::c_str()			const { return static_cast<const Ch*>(data()); }
template<typename Ch> u32 IStringEditor<Ch>::size()					const { return mSize/sizeof(Ch); }
template<typename Ch> u32 IStringEditor<Ch>::capacity()				const { return mCapacity/sizeof(Ch); }
template<typename Ch> bool IStringEditor<Ch>::isDynamic()		        const { return u64(mBuffer) & kDynamicFlag; }
template<typename Ch> bool IStringEditor<Ch>::isEmpty()				const { return !data() || !mSize; }
template<typename Ch> const Ch IStringEditor<Ch>::front()			    const { return data()[0]; }
template<typename Ch> const Ch IStringEditor<Ch>::back()			    const { return data()[size() - 1]; }

template<typename Ch> const Ch* IStringEditor<Ch>::detach()
{
    const Ch* str = data();
    mBuffer = nullptr;
    return str;
}

template<typename Ch>
void IStringEditor<Ch>::allocReserve(u32 charCount)
{
    if (isEmpty() || mEnd)
    {
        return;
    }

    u32 newSize = mSize + charCount;
    if (newSize < mCapacity)
    {
        return;
    }

    if (isDynamic())
    {
        u32 oldCapacity = mCapacity;
        mCapacity = (newSize + 15) & ~15;
        Ch* pNewChars = (Ch*) HeapReallocGlobal( data(), mCapacity * sizeof(Ch) );
        std::memset( data(), 0, oldCapacity * sizeof(Ch) );
        mBuffer = (Ch*)(word(pNewChars) | kDynamicFlag);
        return;
    }

    mEnd = true;
}

template<typename Ch>
void IStringEditor<Ch>::clear()
{
    if (!mBuffer)
        return;

    mEnd = false;
    std::memset(data(), 0, mSize * sizeof(Ch));

    if(mDynamicFlag)
        HeapFreeGlobal( data() );

    mDynamicFlag = NO;
    mBuffer = nullptr;
    mSize = 0;
    mCapacity = 0;
}

template<typename Ch>
IStringEditor<Ch>& IStringEditor<Ch>::appendChar(const Ch c)
{
    if (mEnd)
        return *this;

    allocReserve(1);
    Ch* dst = data();
    dst[mSize++] = c;
    return *this;
}

template<typename Ch>
IStringEditor<Ch>& IStringEditor<Ch>::appendChars(const Ch* str)
{
    if(!str)
        return *this;

    u32 sz = _HelperStrlen<Ch>(str);
    appendChars(str, sz);
    return *this;
}

template<typename Ch>
IStringEditor<Ch>& IStringEditor<Ch>::appendChars(const Ch *begin, u32 size)
{
    if(!begin || !size)
        return *this;

    allocReserve(size);
    if(mEnd)
        return *this;

    Ch* dest = data() + mSize;
    std::memcpy(dest, begin, size * sizeof(Ch));
    mSize += size;
    return *this;
}

template<typename Ch>
IStringEditor<Ch>& IStringEditor<Ch>::appendRange(const Ch *begin, const Ch *end)
{
    if(!begin || !end)
        return *this;

    u32 size = static_cast<u32>(end - begin);
    allocReserve(size);
    if(mEnd)
        return *this;

    Ch* dest = data() + mSize;
    std::memcpy(dest, begin, size * sizeof(Ch));
    mSize += size;
    return *this;
}

template class IStringEditor<char>;
template class IStringEditor<wchar_t>;
}

void tohexstr(const void* blob, size_t blobSize, rw::StringEditor& sb, bool lowercase)
{
	const char* chars = lowercase ? _CHARS_LOWER : _CHARS_UPPER;

	for(size_t i = 0; i < blobSize; i++)
	{
		rw::u8 ch = ((rw::u8*)blob)[i];
		rw::u8 ch_a = rw::u8((ch & 0xf0) >> 4);
		rw::u8 ch_b = rw::u8((ch & 0x0f));
		sb.appendChar(chars[ch_a]);
		sb.appendChar(chars[ch_b]);
	}
}

void wtohexstr(const void* blob, size_t blobSize, rw::WideStringEditor& sb, bool lowercase)
{
	const wchar_t* chars = lowercase ? _WCHARS_LOWER : _WCHARS_UPPER;

	for(size_t i = 0; i < blobSize; i++)
	{
		rw::u8 ch = ((rw::u8*)blob)[i];
		rw::u8 ch_a = rw::u8((ch & 0xf0) >> 4);
		rw::u8 ch_b = rw::u8((ch & 0x0f));
		sb.appendChar(chars[ch_a]);
		sb.appendChar(chars[ch_b]);
	}
}

void fromhexstr(const char* blob, size_t blobSize, void* target, size_t targetSize)
{
	for(size_t i = 0, j = 0; i < blobSize && j < targetSize; i++)
	{
		// chars
		char ch_a = blob[i];
		if(ch_a == '-')
		{
			continue;
		}

		char ch_b = blob[++i];

		// bytes
		rw::u8 b_a   = rw::isalchar(ch_a) ? (rw::islower(ch_a) ? ch_a-0x57 : ch_a-0x37) : ch_a-0x30;
		rw::u8 b_b   = rw::isalchar(ch_b) ? (rw::islower(ch_b) ? ch_b-0x57 : ch_b-0x37) : ch_b-0x30;
		rw::u8 b     = rw::u8((b_a << 4) | b_b);

		((rw::u8*)target)[j++] = b;
	}
}