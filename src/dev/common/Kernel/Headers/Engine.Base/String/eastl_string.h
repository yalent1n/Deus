#pragma once
#include "../Types.h"

namespace eastl
{
	static constexpr unsigned kHeapMask = ~(unsigned(~unsigned(0)) >> 1);
	static constexpr unsigned char c_SSOMask = 0x80;
	
	struct SSOLayout
	{
		static constexpr size_t c_SSOSize				= 16;
		static constexpr size_t c_SSOLengthFieldSize	= 1;
		static constexpr size_t c_SSOBufferSize			= c_SSOSize - c_SSOLengthFieldSize;

		char mData[c_SSOBufferSize];
		char mSize;

		char*				GetLayoutData();
		const char*			GetLayoutData() const;
		
		unsigned int		GetSize() const;
		void				SetSize(unsigned int aSize);

		unsigned int		GetCapacity() const;
		void				SetCapacity(unsigned int aCapacity);		
	};

	struct HeapLayout
	{
		static constexpr unsigned int c_HeapMaxCapacity = ~kHeapMask;

		// Reverse order, because the first byte has the SSO mask.
		char*			mData;
		unsigned int	mSize;
		unsigned int	mCapacity;

		char*				GetLayoutData();
		const char*			GetLayoutData() const;
		void				SetLayoutData(char* data);

		unsigned int		GetSize() const; 
		void				SetSize(unsigned int aSize);

		unsigned int		GetCapacity() const;
		void				SetCapacity(unsigned int aCapacity);
	};

	struct RawLayout
	{
		char mData[sizeof(HeapLayout)];
	};

	struct Layout
	{
		union LayoutData
		{
			SSOLayout		sso;
			HeapLayout		heap;
			RawLayout		raw;
		};

		bool						IsHeap() const;
		bool						IsSSO() const;

		unsigned int				GetSize() const;
		void						SetSize(unsigned int aSize);

		unsigned int				GetCapacity() const;
		void						SetCapacity(unsigned int aCapacity);

		char*						GetBeginPtr();
		const char*					GetBeginPtr() const;

		char*						GetEndPtr();
		const char*					GetEndPtr() const;
		
		void						SetData(char* aBeginPtr, char* aEndPtr);
		void						CopyData(const char* aBeginPtr, const char* aEndPtr);
		
		LayoutData mData;
	};
	
	class string
	{
	public:
		typedef char value_type;
		typedef unsigned int size_type;
		
		static constexpr size_type npos = size_type(-1);

		LIB_LINKAGE string();
		LIB_LINKAGE string(const value_type* aData);
		LIB_LINKAGE string(const value_type* aData, size_type aSize);
		LIB_LINKAGE string(const string& aOther);
		~string() = default;

		
		LIB_LINKAGE const value_type*		c_str() const;
		LIB_LINKAGE value_type*				data();
		LIB_LINKAGE size_type				size() const;
		LIB_LINKAGE size_type				capacity() const;
		LIB_LINKAGE bool					empty() const;

		LIB_LINKAGE const char				operator[](size_type index) const;
		LIB_LINKAGE string&					operator=(const string& other);
		LIB_LINKAGE string&					operator=(const  char* str);
		LIB_LINKAGE bool					operator==(const string& other) const;
		LIB_LINKAGE 						operator const char*() const;

	private:
		Layout mLayout;
	};
}