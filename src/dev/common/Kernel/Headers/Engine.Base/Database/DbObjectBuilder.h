#pragma once
#include "../Bundle.h"
#include "../Memory/VirtualAlloc.h"
#include "../String/String.h"
#include "../Misc/Time.h"
#include "../Misc/Json.h"
#include "../Buffer/Adapter.h"
#include "DbObject.h"

#define DBOBJ(x) ((rw::DbObjectBuilder() << x).done())
#define DBARR(x) ((rw::DbObjectArrayBuilder() << x).done())
#define TDBOBJ(x) ((rw::FixedDbObjectBuilder<>() << x).done())
#define TDBARR(x) ((rw::FixedDbObjectArrayBuilder<>() << x).done())

namespace rw
{
class DbObjectStream;
class DbObjectBuilder;
class DbObjectArrayBuilder;

FB_BASE_API void JSON_toPrototypeObject(json_object* rootObj, DbObjectBuilder& builder);
FB_BASE_API void JSON_toPrototypeArray(json_object* rootObj, DbObjectArrayBuilder& builder);

class DbObjectStream
{
public:
	inline DbObjectStream(DbObjectBuilder* builder)
		: m_fieldName(nullptr)
		, m_builder(builder)
	{}

	inline ~DbObjectStream()
	{
		m_fieldName = nullptr;
		m_builder = nullptr;
	}
	
	inline void newField(const char* name) { m_fieldName = name; }
	
	template<typename T> 
	DbObjectBuilder& operator<<(const T& fieldValue) const;
	
private:
	const char* m_fieldName;
	DbObjectBuilder* m_builder;
};

class DbObjectBuilder
{
protected:
	enum { kMaxHeaderSize = 7 };
	
	FB_BASE_API 	 DbObjectBuilder(const char* name, u8* workspace, u32 workspaceSize, u8* doneBuf, u32 doneBufSize);
	
public:
	FB_BASE_API 	 DbObjectBuilder(const char* name = nullptr);
	FB_BASE_API 	~DbObjectBuilder();

	inline DbObjectBuilder(const DbObjectBuilder& other)
		: m_name(other.m_name)
		, m_builderStream(this)
		, m_buffer(other.m_buffer)
		, m_subObjectBuffer(other.m_subObjectBuffer)
	{}
	inline DbObjectBuilder(DbObjectBuilder&& other)
		: m_name(other.m_name)
		, m_builderStream(this)
		, m_buffer(other.m_buffer)
		, m_subObjectBuffer(other.m_subObjectBuffer)
	{
		other.m_name = NO;
		other.m_builderStream.~DbObjectStream();
		memset(&other.m_buffer, 0, sizeof(AllocatingBuffer));
		memset(&other.m_subObjectBuffer, 0, sizeof(AllocatingBuffer));
	}
	
	FB_BASE_API virtual DbObject		done();
	FB_BASE_API virtual DbObjectScatter doneScatter();
	
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, bool value);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, const StringEditor& value);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, const char* value);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, const wchar_t* value);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, CVarInt num);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, s32 num);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, u32 num);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, s64 num);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, u64 num);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, f32 num);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, f64 num);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, DateTime time);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, const Guid& guid);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, const SHA1& digest);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, const SHA256& digest);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, const PropData& blob);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, const DbOid& oid);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, CNull);
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, const void* blob, u64 blobSize);
	
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, const DbObject& o);		
	FB_BASE_API DbObjectBuilder&		append(const char* fieldName, const DbObjectElement& e);
	FB_BASE_API DbObjectBuilder&		appendNull(const char* fieldName);
	
	FB_BASE_API DbObjectBuilder&		appendElements(const DbObject& o);
	FB_BASE_API DbObjectBuilder&		appendElement(const DbObjectElement& e);			

	inline DbObjectStream& operator<<(const char* fieldName)
	{
		m_builderStream.newField(fieldName);
		return m_builderStream;
	}

	inline DbObjectBuilder& operator=(DbObjectBuilder&& other)
	{
		m_name = other.m_name;
		other.m_name = NO;
		m_builderStream = this;
		other.m_builderStream.~DbObjectStream();
		m_buffer = other.m_buffer;
		memset(&other.m_buffer, 0, sizeof(AllocatingBuffer));
		m_subObjectBuffer = other.m_subObjectBuffer;
		memset(&other.m_subObjectBuffer, 0, sizeof(AllocatingBuffer));
		return *this;
	}
	
protected:
	FB_BASE_API void					internalAppend(DbTypesEnum type, const char* fieldName);
	FB_BASE_API DbObject				internalDone(DbTypesEnum type);
	FB_BASE_API DbObjectScatter			internalDoneScatter(DbTypesEnum type);
	
	const char* m_name;
	DbObjectStream m_builderStream;
	AllocatingBuffer m_buffer;
	AllocatingBuffer m_subObjectBuffer;
};

template<typename T>
inline DbObjectBuilder& DbObjectStream::operator<<(const T& fieldValue) const
{
	m_builder->append(m_fieldName, fieldValue);
	return *m_builder;
}

template<u32 Size = 1024>
class FixedDbObjectBuilder : public DbObjectBuilder
{
	u8 m_workspace[Size]{0};
	
public:
	inline FixedDbObjectBuilder(const char* name = nullptr)
		: DbObjectBuilder(name, m_workspace, Size, (u8*) HeapAllocGlobal(Size + kMaxHeaderSize + strlen(name)), Size + kMaxHeaderSize + strlen(name))
	{}
};

class DbObjectArrayBuilder : public DbObjectBuilder
{
protected:
	DbObjectArrayBuilder(const char* name, u8* workspace, u32 workspaceSize, u8* doneBuf, u32 doneBufSize)
		: DbObjectBuilder(name, workspace, workspaceSize, doneBuf, doneBufSize)
	{}

public:
	DbObjectArrayBuilder(const char* name = nullptr)
		: DbObjectBuilder(name)
	{}

	inline DbObject done() override
	{
		return this->internalDone(kDbType_Array);
	}

	inline DbObjectScatter doneScatter() override
	{
		return this->internalDoneScatter(kDbType_Array);
	}
	
	template<typename T>
	DbObjectArrayBuilder& operator<<(const T& value)
	{
		append("", value); 
		return *this;
	}
};

template<u32 Size = 1024>
class FixedDbObjectArrayBuilder : public DbObjectArrayBuilder
{
	u8 m_workspace[Size]{0};
public:
	FixedDbObjectArrayBuilder(const char* name = nullptr)
		: DbObjectArrayBuilder(name, m_workspace, Size, (u8*) HeapAllocGlobal(Size + kMaxHeaderSize + strlen(name)), Size + kMaxHeaderSize + strlen(name))
	{}
};
}