#include <Engine.Base/Database/DbObjectBuilder.h>

namespace rw
{
namespace jsonInternal
{
void build(json_object* rootObj, DbObjectBuilder* builder, bool isArray)
{
	if(!rootObj)
		return;

	if(isArray)
	{
		EASSERT(rootObj->type == json_object::eTypeArray, "Attempted to convert a JSON json_object to an array");
	}
	else
	{
		EASSERT(rootObj->type == json_object::eTypeObject, "Attempted to convert a JSON item to an object");
	}

	json_object* child = rootObj->child;
	while(child)
	{
		const char* name = child->key ? child->key : "";

		switch(child->type)
		{
		case json_object::eTypeString:
			builder->append(name, child->valuestring);
			break;
		case json_object::eTypeInteger:
			{
				u32 truncated = u32(child->valueint);
				if(truncated == child->valueint)
					builder->append(name, truncated);
				else
					builder->append(name, u64(child->valueint));

				break;
			}
		case json_object::eTypeDouble:
			{
				float truncated = float(child->valuedouble);
				if(truncated == child->valuedouble)
					builder->append(name, truncated);
				else
					builder->append(name, child->valuedouble);
					
				break;
			}
		case json_object::eTypeBoolean:
			builder->append(name, child->valueboolean);
			break;
		case json_object::eTypeNull:
			builder->append(name, CNull{});
			break;
		case json_object::eTypeObject:
			{
				DbObjectBuilder subbuilder;
				build(child, &subbuilder, false);
				builder->append(name, subbuilder.done());
				break;
			}
		case json_object::eTypeArray:
			{
				DbObjectArrayBuilder subbuilder;
				build(child, &subbuilder, true);
				builder->append(name, subbuilder.done());
				break;
			}
		case json_object::eTypeUndefined:
			EASSERT(false, "Undefined json item type");
		}
		child = child->sibling;
	}
}
}
	
void JSON_toPrototypeObject(json_object* rootObj, DbObjectBuilder& builder)
{
	jsonInternal::build(rootObj, &builder, false);
}

void JSON_toPrototypeArray(json_object* rootObj, DbObjectArrayBuilder& builder)
{
	jsonInternal::build(rootObj, &builder, true);
}
	
DbObjectBuilder::DbObjectBuilder(const char* name)
	: m_name(name)
	, m_builderStream(this)
	, m_buffer()
	, m_subObjectBuffer()
{}

DbObjectBuilder::DbObjectBuilder(const char* name, u8* workspace, u32 workspaceSize, u8* doneBuf, u32 doneBufSize)
	: m_name(name)
	, m_builderStream(this)
	, m_buffer(doneBuf, doneBufSize)
	, m_subObjectBuffer(workspace, workspaceSize)
{}

DbObjectBuilder::~DbObjectBuilder()
{
	m_name = nullptr;
	// buffers close automatically using destructor.	
}

void
DbObjectBuilder::internalAppend(DbTypesEnum type, const char* fieldName)
{
	if (isnil(fieldName))
	{
		m_subObjectBuffer.write(u8(type | kDbType_Anonymous));
		return;
	}

	m_subObjectBuffer.write(u8(type));
	m_subObjectBuffer.encodeString(fieldName);
}

DbObject DbObjectBuilder::internalDone(DbTypesEnum type)
{
	m_subObjectBuffer.write(u8(kDbType_Eoo));
	u32 size = static_cast<u32>(m_subObjectBuffer.getPosition());
	m_subObjectBuffer.setPosition(0);
	
	u8 typeFlags = type | (m_name ? kDbType_Eoo : kDbType_Anonymous);
	m_buffer.setPosition(0);
	m_buffer.write(typeFlags);

	if (m_name)
		m_buffer.encodeString(m_name);

	m_buffer.encodeUint(size);
	m_subObjectBuffer.copyTo(m_buffer, size);

	u8* result = (u8*) m_buffer.detach();
	m_builderStream.~DbObjectStream();
	m_buffer.flush();
	m_subObjectBuffer.flush();

	return { result, DatabaseOwnershipPolicy::Assume };
}

DbObjectScatter DbObjectBuilder::internalDoneScatter(DbTypesEnum type)
{
	m_subObjectBuffer.write(u8(kDbType_Eoo));
	u32 size = static_cast<u32>(m_subObjectBuffer.position());
	u8* value = (u8*) m_subObjectBuffer.bytes();

	u8 typeFlags = type | (m_name ? kDbType_Eoo : kDbType_Anonymous);
	m_buffer.seek(0, E_BIO_SEEK_START);
	m_buffer.write(typeFlags);

	if (m_name)
		m_buffer.encodeString(m_name);

	m_buffer.encodeUint(size);
	m_buffer.write(value, size);

	u32 dataSize = m_buffer.position();
	u8* data = (u8*) m_buffer.detach();
	m_builderStream.~DbObjectStream();
	m_buffer.flush();
	m_subObjectBuffer.flush();

	return { data, dataSize };
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, bool value)
{
	internalAppend(kDbType_Bool, fieldName);
	m_subObjectBuffer.write(value ? u8(1) : u8(0));
	return *this;
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, const StringEditor& value)
{
	if(!value.isEmpty())
	{
		internalAppend(kDbType_String, fieldName);
		m_subObjectBuffer.encodeUint(u64(value.size()) + 1);
		m_subObjectBuffer.encodeString(value.c_str());
	}
	return *this;
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, const char* value)
{
	if(value)
	{
		internalAppend(kDbType_String, fieldName);
		m_subObjectBuffer.encodeUint(strlen(value) + 1);
		m_subObjectBuffer.encodeString(value);
	}
	return *this;
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, const wchar_t* value)
{
	if(value)
	{
		internalAppend(kDbType_String, fieldName);
		m_subObjectBuffer.encodeUint(u64(wstrlen(value)) + 1);
		m_subObjectBuffer.encodeWString(value);
	}
	return *this;
}

DbObjectBuilder&
DbObjectBuilder::append(const char* fieldName, CVarInt num)
{
	internalAppend(kDbType_VarInt, fieldName);
	m_subObjectBuffer.encodeUint64(num.value);
	return *this;
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, s32 num)
{
	internalAppend(kDbType_Integer, fieldName);
	m_subObjectBuffer.write(num);
	return *this;
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, u32 num)
{
	internalAppend(kDbType_Integer, fieldName);
	m_subObjectBuffer.write(num);
	return *this;
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, s64 num)
{
	internalAppend(kDbType_Long, fieldName);
	m_subObjectBuffer.write(num);
	return *this;
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, u64 num)
{
	internalAppend(kDbType_Long, fieldName);
	m_subObjectBuffer.write(num);
	return *this;
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, f32 num)
{
	internalAppend(kDbType_Float, fieldName);
	m_subObjectBuffer.write(num);
	return *this;
}

DbObjectBuilder&
DbObjectBuilder::append(const char* fieldName, f64 num)
{
	internalAppend(kDbType_Double, fieldName);
	m_subObjectBuffer.write(num);
	return *this;
}

DbObjectBuilder& DbObjectBuilder::append(const char* fieldName, DateTime time)
{
	internalAppend(kDbType_Timestamp, fieldName);
	m_subObjectBuffer.write(time.toTimestamp());
	return *this;
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, const Guid& guid)
{
	internalAppend(kDbType_Guid, fieldName);
	m_subObjectBuffer.write(&guid, sizeof(Guid));
	return *this;
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, const SHA1& digest)
{
	internalAppend(kDbType_Sha1, fieldName);
	m_subObjectBuffer.write(&digest, sizeof(SHA1));
	return *this;
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, const SHA256& digest)
{
	internalAppend(kDbType_Sha256, fieldName);
	m_subObjectBuffer.write(&digest, sizeof(SHA256));
	return *this;
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, const PropData& blob)
{
	internalAppend(kDbType_Blob, fieldName);
	m_subObjectBuffer.encodeUint(blob.size());
	m_subObjectBuffer.write(blob.data(), blob.size());
	return *this;
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, const DbOid& oid)
{
	internalAppend(kDbType_ObjectId, fieldName);
	m_subObjectBuffer.write(&oid, sizeof(DbOid));
	return *this;
}

DbObjectBuilder&
DbObjectBuilder::append(const char* fieldName, CNull)
{
	internalAppend(kDbType_Null, fieldName);
	return *this;
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, const void* blob, u64 blobSize)
{
	internalAppend(kDbType_Blob, fieldName);
	m_subObjectBuffer.encodeUint(blobSize);
	m_subObjectBuffer.write(blob, blobSize);
	return *this;
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, const DbObjectElement& e)
{
	const u8* ptr = e.value();
	u32 size = e.size();
	internalAppend(e.type(), fieldName);

	if (e.isDbVariant() || e.isBlob() || e.isString())
	{
		const u8* original_ptr = ptr;
		ptr = e.payload() + 1;
		if (!e.isAnonymous())
		{
			while (*ptr++)
				;
		}

		size += u32(original_ptr - ptr);
	}

	if (ptr)
		m_subObjectBuffer.write(ptr, size);

	return *this;
}

DbObjectBuilder&		
DbObjectBuilder::append(const char* fieldName, const DbObject& o)
{
	//ELOGF("Append %s as db object", fieldName);

	if(o.isNull())
	{
		append(fieldName, CNull{});
		return *this;
	}
	
	internalAppend(o.isArray() ? kDbType_Array : kDbType_Object, fieldName);
	const u8* valueBuf = o.tableData();	
	u32	size = o.tableSizeBytes();
	//ELOGF("%s is an %s located at %p with the size of %u bytes", fieldName, (o.isArray() ? "array" : "object"), valueBuf, size);
	m_subObjectBuffer.encodeUint(size);
	m_subObjectBuffer.write(valueBuf, size);
	return *this;
}

DbObjectBuilder& DbObjectBuilder::appendNull(const char* fieldName)
{
	return append(fieldName, CNull{});
}

DbObjectBuilder& DbObjectBuilder::appendElements(const DbObject& o)
{
	DbObjectElementIterator it(o);

	while(it.more())
	{
		DbObjectElement e = it.next();
		appendElement(e);
	}

	return *this;
}

DbObjectBuilder& DbObjectBuilder::appendElement(const DbObjectElement& e)
{
	return append(e.name(), e);
}

DbObject
DbObjectBuilder::done()
{
	return internalDone(kDbType_Object);
}

DbObjectScatter
DbObjectBuilder::doneScatter()
{
	return internalDoneScatter(kDbType_Object);
}
}