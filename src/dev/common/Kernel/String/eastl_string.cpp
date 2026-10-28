#include <Engine.Base/String/eastl_string.h>
#include <Engine.Base/Log.h>
#include <Engine.Base/Memory/VirtualAlloc.h>

namespace eastl 
{
	char* SSOLayout::GetLayoutData()
	{
		return mData;
	}
	
	const char* SSOLayout::GetLayoutData() const
	{
		return mData;
	}
	
	unsigned int SSOLayout::GetSize() const
	{
		return mSize;
	}

	void SSOLayout::SetSize(unsigned int aSize)
	{
		__builtin_assert(aSize <= c_SSOBufferSize);
		mSize = aSize;		
	}

	unsigned int SSOLayout::GetCapacity() const
	{
		return c_SSOBufferSize;
	}
	
	void SSOLayout::SetCapacity(unsigned int aCapacity)
	{
	}

	char* HeapLayout::GetLayoutData()
	{
		return mData;
	}

	const char*	HeapLayout::GetLayoutData() const
	{
		return mData;
	}

	void HeapLayout::SetLayoutData(char* data)
	{
		mData = data;
	}
	
	unsigned int HeapLayout::GetSize() const
	{
		return mSize;
	}
	
	void HeapLayout::SetSize(unsigned int aSize)
	{
		__builtin_assert(aSize <= c_HeapMaxCapacity);
		mSize = aSize;
	}

	unsigned int HeapLayout::GetCapacity() const
	{
		return (mCapacity & ~kHeapMask);
	}
	
	void HeapLayout::SetCapacity(unsigned int aCapacity)
	{
		__builtin_assert(aCapacity <= c_HeapMaxCapacity);
		mCapacity = (aCapacity | kHeapMask);
	}

	bool Layout::IsHeap() const
	{ return !!(mData.sso.mSize & c_SSOMask); }
	
	bool Layout::IsSSO() const
	{ return !IsHeap(); }
		
	unsigned int Layout::GetSize() const
	{ return IsSSO() ? mData.sso.GetSize() : mData.heap.GetSize(); }
		
	void Layout::SetSize(unsigned int aSize)
	{
		if(aSize < SSOLayout::c_SSOBufferSize)
		{
			mData.sso.SetSize(aSize);
			return;
		}
		
		mData.heap.SetSize(aSize);
	}

	unsigned int Layout::GetCapacity() const
		{ return IsSSO() ? mData.sso.GetCapacity() : mData.heap.GetCapacity(); }
		
	void Layout::SetCapacity(unsigned int aCapacity)
		{ IsSSO() ? mData.sso.SetCapacity(aCapacity) : mData.heap.SetCapacity(aCapacity); }

	char* Layout::GetBeginPtr()
		{ return IsSSO() ? mData.sso.GetLayoutData() : mData.heap.GetLayoutData(); }

	const char* Layout::GetBeginPtr() const
		{ return IsSSO() ? mData.sso.GetLayoutData() : mData.heap.GetLayoutData(); }

	char* Layout::GetEndPtr()
		{
			return IsSSO()
			? mData.sso.GetLayoutData() + mData.sso.GetSize()
			: mData.heap.GetLayoutData() + mData.heap.GetSize();
		}
	const char* Layout::GetEndPtr() const
		{
			return IsSSO()
			? mData.sso.GetLayoutData() + mData.sso.GetSize()
			: mData.heap.GetLayoutData() + mData.heap.GetSize();
		}

	// NOTE:
	// The reason why SetData & CopyData SSO
	// copying is not Less-or-Equal is because the NULL character
	// would be outside the 16-byte boundary,
	// and stack corruption would happen. :) 
	
	void Layout::SetData(char* aBeginPtr, char* aEndPtr)
	{
		__builtin_assert(aBeginPtr != nullptr);
		__builtin_assert(aEndPtr != nullptr);

		if(aBeginPtr == GetBeginPtr() && aEndPtr == GetEndPtr())
			return;

		unsigned int size = static_cast<unsigned int>(aEndPtr - aBeginPtr);
		// If the size contains the NULL character, then
		// maybe it's from a stack/static allocated char array!
		if(aBeginPtr[size - 1] == '\0') {
			size -= 1;
		}
		
		if(size < SSOLayout::c_SSOBufferSize) {
			mData.sso.SetSize(size);
			memcpy_s(mData.sso.GetLayoutData(), SSOLayout::c_SSOBufferSize, aBeginPtr, SSOLayout::c_SSOBufferSize);
			mData.sso.GetLayoutData()[size] = '\0';
		}
		else {
			mData.heap.SetSize(size);
			mData.heap.SetCapacity(size);
			mData.heap.mData = aBeginPtr;
		}
	}

	void Layout::CopyData(const char* aBeginPtr, const char* aEndPtr)
	{
		__builtin_assert(aBeginPtr != nullptr);
		__builtin_assert(aEndPtr != nullptr);

		if(aBeginPtr == GetBeginPtr() && aEndPtr == GetEndPtr())
			return;
		
		unsigned int size = static_cast<unsigned int>(aEndPtr - aBeginPtr);
		if(size == 0) {
			mData.sso.SetSize(0);
			memset(mData.sso.mData, 0, SSOLayout::c_SSOBufferSize);
			return;
		}

		// If the size contains the NULL character, then
		// maybe it's from a stack/static allocated char array!
		if(aBeginPtr[size - 1] == '\0') {
			size -= 1;
		}
		
		if(size < SSOLayout::c_SSOBufferSize) {
			mData.sso.SetSize(size);
			memcpy_s(mData.sso.GetLayoutData(), SSOLayout::c_SSOBufferSize, aBeginPtr, SSOLayout::c_SSOBufferSize);
			mData.sso.GetLayoutData()[size] = '\0';
		}
		else {
			mData.heap.SetSize(size);
			mData.heap.SetCapacity(size);

			
			char* pHeap = static_cast<char*>(rw::HeapAllocGlobal(size + 1));
			memcpy_s(pHeap, size, aBeginPtr, size);
			pHeap[size] = '\0';
			mData.heap.mData = pHeap;
		}
	}
	
	string::string()
	{
		memset(&mLayout, 0, sizeof(Layout));
	}

	string::string(const value_type* aData)
		: mLayout()
	{
		mLayout.CopyData(aData, aData + rw::strlen(aData));
	}
	
	string::string(const value_type* aData, size_type aSize)
		: mLayout()
	{
		mLayout.CopyData(aData, aData + aSize);
	}
	
	string::string(const string& aOther)
	{
		mLayout = aOther.mLayout;
	}
	
	const string::value_type* string::c_str() const
	{
		return mLayout.GetBeginPtr();
	}

	string::value_type* string::data()
	{
		return mLayout.GetBeginPtr();
	}	

	string::size_type string::size() const
	{
		return mLayout.GetSize();
	}

	string::size_type string::capacity() const
	{
		return mLayout.GetCapacity();
	}
	
	bool string::empty() const
	{
		return mLayout.GetSize() == 0;
	}

	const char string::operator[](size_type index) const
	{
		__builtin_assert(index < mLayout.GetSize());
		return mLayout.GetBeginPtr()[index];
	}
	string& string::operator=(const string& other)
	{
		mLayout = other.mLayout;
		return *this;
	}
	string& string::operator=(const char* str)
	{
		mLayout.CopyData(str, str + rw::strlen(str));
		return *this;
	}

	bool string::operator==(const string& other) const
	{
		return rw::equalsCase(mLayout.GetBeginPtr(), other.mLayout.GetBeginPtr());
	}

	string::operator const char*() const
	{
		return mLayout.GetBeginPtr();
	}
}