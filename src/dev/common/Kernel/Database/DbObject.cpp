#include <Engine.Base/Database/DbObject.h>
#include <Engine.Base/Database/DbObjectBuilder.h>
#include <Engine.Base/Memory/VirtualAlloc.h>
#include <Engine.Base/Crypto/Base64.h>
#include <Engine.Base/Misc/Util.h>
#include <cstring>

#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wswitch"
#endif

namespace rw
{
const char* tostring(DbTypesEnum type)
{
	switch (type)
	{
	case kDbType_Eoo:
		return "End of object terminator";
	case kDbType_Array:
		return "Array";
	case kDbType_Object:
		return "Object";
	case kDbType_HomoArray:
		return "Homogeneous array";
	case kDbType_Null:
		return "Null";
	case kDbType_ObjectId:
		return "Unique object identifier";
	case kDbType_Bool:
		return "Boolean";
	case kDbType_String:
		return "String";
	case kDbType_Integer:
		return "32-bit integer";
	case kDbType_Long:
		return "64-bit integer";
	case kDbType_VarInt:
		return "Variable-length encoded integer";
	case kDbType_Float:
		return "32-bit floating point";
	case kDbType_Double:
		return "64-bit floating point";
	case kDbType_Timestamp:
		return "64-bit timestamp";
	case kDbType_Guid:
		return "Guid";
	case kDbType_Sha1:
		return "Sha1";
	case kDbType_Sha256:
		return "Sha256";
	case kDbType_Matrix44:
		return "4x4 float matrix";
	case kDbType_Vector4:
		return "4 float vector";;
	case kDbType_Blob:
		return "Binary blob";
	case kDbType_Attachment:
		return "Attachment";
	case kDbType_Timespan:
		return "Timespan (duration)";
	default:
		return "Undefined";
	}
}

const char* 
DbObjectElement::name() const
{
	if(isEoo())
		return NULL;
	return isAnonymous() 
		? NULL 
		: reinterpret_cast<const char*>(m_payload + 1);
}

DbTypesEnum 
DbObjectElement::type() const
{
	return DbTypesEnum(m_payload[0] & kDbType_Mask);
}

u32 
DbObjectElement::objectSize() const
{
	const u8* ptr = m_payload + 1;

	if (!isAnonymous())
	{
		while (*ptr++)
			;
	}

	if (isDbVariant() || isString() || isBlob())
		return decodeVarUint(ptr);

	return 0;
}

bool DbObjectElement::isNull() const
{
	if(m_payload == nullptr)
		return true;

	return type() == kDbType_Null || type() == kDbType_Eoo;
}
	
bool DbObjectElement::isNullOrEmpty() const
{
	if(isNull())
		return true;

	DbTypesEnum type = static_cast<DbTypesEnum>(m_payload[0] & kDbType_Mask);

	if(type == kDbType_Object || type == kDbType_Array || type == kDbType_Blob || type ==kDbType_String)
		return objectSize() == 0;

	return false;
}
	
u32 
DbObjectElement::size() const
{
	switch (type())
	{
	case kDbType_Bool:
		return 1;
	case kDbType_Array:
	case kDbType_Object:
	case kDbType_String:
	case kDbType_Blob:
		return objectSize();
	case kDbType_Integer:
	case kDbType_Float:
		return sizeof(u32);
	case kDbType_Timestamp:
	case kDbType_Long:
	case kDbType_Double:
		return sizeof(u64);
	case kDbType_ObjectId:
		return sizeof(DbOid);
	case kDbType_Guid:
		return sizeof(Guid);
	case kDbType_Sha1:
	case kDbType_Attachment:
		return sizeof(SHA1);
	case kDbType_Sha256:
		return sizeof(SHA256);
	case kDbType_Vector4:
		return 16;
	case kDbType_Matrix44:
		return 16 * 4;
	default:
		return 0;
	}
}

u8*
DbObjectElement::payload() const
{
	return m_payload;
}

u8*
DbObjectElement::value() const
{
	// if (isNull())
	// 	return nullptr;

	// skip the flags & type
	u8* ptr = payload() + 1;

	if (!isAnonymous())
	{
		while (*ptr++)
			;
	}

	if (isDbVariant() || isString() || isBlob()) {
		ptr += decodeSize(ptr);
	}

	return ptr;
}

const char* 
DbObjectElement::valueString(const char* defaultValue) const
{
	if (isNullOrEmpty())
		return defaultValue;

	return (const char*) value();
}

PropData 
DbObjectElement::valueBlob(PropData defaultValue) const 
{ 
	if (isNullOrEmpty())
		return defaultValue;

	return PropData(value(), objectSize(), DatabaseOwnershipPolicy::Shared);
}

const DbOid&
DbObjectElement::valueOid(const DbOid& defaultValue) const
{
	if(isNull())
		return defaultValue;

	return *(DbOid*) value();
}

u32 
DbObjectElement::valueInteger(u32 defaultValue) const 
{ 
	if (isNull())
		return defaultValue;

	return *(u32*) value();
}

u64
DbObjectElement::valueLong(u64 defaultValue) const 
{ 
	if (isNull())
		return defaultValue;

	return *(u64*) value();
}

CVarInt DbObjectElement::valueVarInt(CVarInt defaultValue) const
{
	if(isNull())
		return defaultValue;

	const void* ptr = value();
	#ifdef SYSARCH_X86
	return decodeVarUint(ptr);
	#else
	return decodeVarUint64(ptr);
	#endif
}

f32 
DbObjectElement::valueFloat(f32 defaultValue) const 
{ 
	if (isNull())
		return defaultValue;

	return *(f32*) value();
}

f64 
DbObjectElement::valueDouble(f64 defaultValue) const 
{ 
	if (isNull())
		return defaultValue;

	return *(f64*) value();
}

const Guid& 
DbObjectElement::valueGuid(const Guid& defaultValue) const 
{ 
	if (isNull())
		return defaultValue;

	return *(Guid*) value(); 
}

const SHA1&
DbObjectElement::valueSha1(const SHA1& defaultValue) const 
{ 
	if (isNull())
		return defaultValue;

	return *(SHA1*) value();
}

const SHA256&
DbObjectElement::valueSha256(const SHA256& defaultValue) const 
{ 
	if (isNull())
		return defaultValue;

	return *(SHA256*) value();
}

const DateTime& DbObjectElement::valueTime(const DateTime& defaultValue) const
{
	if(isNull())
		return defaultValue;

	return *(DateTime*) value();
}

bool
DbObjectElement::valueBool(bool defaultValue) const 
{ 
	if (isNull())
		return defaultValue;

	return *value() != 0; 
}

DbObject 
DbObjectElement::valueObject() const
{
	if (isNullOrEmpty())
		return { nullptr };

	return { m_payload, DatabaseOwnershipPolicy::Shared };
}

DbObjectArray 
DbObjectElement::valueArray() const
{
	if (isNullOrEmpty())
		return nullptr;

	return { m_payload, DatabaseOwnershipPolicy::Shared };
}

bool DbObjectElement::getValueSafe(bool &outValue) const
{
	if(isNullOrEmpty())
		return false;
	
	switch(type())
	{
		case kDbType_Integer:
			outValue = valueInteger() != 0;
			return true;
		case kDbType_Long:
			outValue = valueLong() != 0;
			return true;
		case kDbType_Bool:
			outValue = valueBool();
			return true;
		case kDbType_Float:
			outValue = valueFloat() != 0.0f;
			return true;
		case kDbType_Double:
			outValue = valueDouble() != 0.0f;
			return true;
		case kDbType_VarInt:
			outValue = valueVarInt().value != 0;
			return true;
		case kDbType_String:
			outValue = equalsIgnoreCase("true", valueString(), 4);
			return true;
	}

	return false;
}

bool DbObjectElement::getValueSafe(s32 &outValue) const
{
	if(isNullOrEmpty())
		return false;
	
	switch(type())
	{
		case kDbType_Integer:
			outValue = static_cast<int>(valueInteger());
			return true;
		case kDbType_Long:
			outValue = static_cast<int>(valueLong());
			return true;
		case kDbType_VarInt:
			outValue = static_cast<int>(valueVarInt().value);
			return true;
		case kDbType_Float:
			outValue = static_cast<int>(valueFloat());
			return true;
		case kDbType_Double:
			outValue = static_cast<int>(valueDouble());
			return true;
		case kDbType_Bool:
			outValue = static_cast<int>(valueBool());
			return true;
	}

	return false;
}

bool DbObjectElement::getValueSafe(u32 &outValue) const
{
	if(isNullOrEmpty())
		return false;
	
	switch(type())
	{
	case kDbType_Integer:
		outValue = valueInteger();
		return true;
	case kDbType_Long:
		outValue = static_cast<u32>(valueLong());
		return true;
	case kDbType_VarInt:
		outValue = static_cast<u32>(valueVarInt().value);
		return true;
	case kDbType_Float:
		outValue = static_cast<u32>(valueFloat());
		return true;
	case kDbType_Double:
		outValue = static_cast<u32>(valueDouble());
		return true;
	case kDbType_Bool:
		outValue = static_cast<u32>(valueBool());
		return true;
	}

	return false;
}

bool DbObjectElement::getValueSafe(s64 &outValue) const
{
	if(isNullOrEmpty())
		return false;
	
	switch(type())
	{
	case kDbType_Integer:
		outValue = static_cast<s64>(valueInteger());
		return true;
	case kDbType_Long:
		outValue = static_cast<s64>(valueLong());
		return true;
	case kDbType_VarInt:
		outValue = static_cast<s64>(valueVarInt().value);
		return true;
	case kDbType_Float:
		outValue = static_cast<s64>(valueFloat());
		return true;
	case kDbType_Double:
		outValue = static_cast<s64>(valueDouble());
		return true;
	case kDbType_Bool:
		outValue = static_cast<s64>(valueBool());
		return true;
	}

	return false;
}

bool DbObjectElement::getValueSafe(u64 &outValue) const
{
	if(isNullOrEmpty())
		return false;
	
	switch(type())
	{
	case kDbType_Integer:
		outValue = static_cast<u64>(valueInteger());
		return true;
	case kDbType_Long:
		outValue = static_cast<u64>(valueLong());
		return true;
	case kDbType_VarInt:
		outValue = static_cast<u64>(valueVarInt().value);
		return true;
	case kDbType_Float:
		outValue = static_cast<u64>(valueFloat());
		return true;
	case kDbType_Double:
		outValue = static_cast<u64>(valueDouble());
		return true;
	case kDbType_Bool:
		outValue = static_cast<u64>(valueBool());
		return true;
	}

	return false;
}

bool DbObjectElement::getValueSafe(float &outValue) const
{
	if(isNullOrEmpty())
		return false;
	
	switch(type())
	{
	case kDbType_Integer:
		outValue = static_cast<float>(valueInteger());
		return true;
	case kDbType_Long:
		outValue = static_cast<float>(valueLong());
		return true;
	case kDbType_Float:
		outValue = valueFloat();
		return true;
	case kDbType_Double:
		outValue = static_cast<double>(valueDouble());
		return true;
	case kDbType_Bool:
		outValue = static_cast<float>(valueBool());
		return true;
	}

	return false;
}

bool DbObjectElement::getValueSafe(double &outValue) const
{
	if(isNullOrEmpty())
		return false;
	
	switch(type())
	{
	case kDbType_Integer:
		outValue = static_cast<double>(valueInteger());
		return true;
	case kDbType_Long:
		outValue = static_cast<double>(valueLong());
		return true;
	case kDbType_Float:
		outValue = static_cast<double>(valueFloat());
		return true;
	case kDbType_Double:
		outValue = valueDouble();
		return true;
	case kDbType_Bool:
		outValue = static_cast<double>(valueBool());
		return true;
	}

	return false;
}

bool DbObjectElement::getValueSafe(StringEditor& outValue) const
{
	if(isNullOrEmpty())
		return false;
	
	switch(type())
	{
	case kDbType_Null:
		outValue << "null";
		return true;
	case kDbType_ObjectId:
		valueOid().toString(outValue);
		return true;
	case kDbType_Bool:
		outValue << (valueBool() ? "true" : "false");
		return true;
	case kDbType_String:
		outValue.appendChars(valueString());
		return true;
	case kDbType_Integer:
		tostring(valueInteger(), outValue);
		return true;
	case kDbType_Long:
		tostring(valueLong(), outValue);
		return true;
	case kDbType_Timestamp:
		valueTime().toString(outValue);
		return true;
	case kDbType_VarInt:
		tostring(valueVarInt().value, outValue);
		return true;
	case kDbType_Float:
		tostring(valueFloat(), outValue);
		return true;
	case kDbType_Double:
		tostring(valueDouble(), outValue);
		return true;
	case kDbType_Guid:
		outValue.appendChars(GuidStr(valueGuid()).str);		
		return true;
	case kDbType_Sha1:
		outValue.appendChars(Sha1Str(valueSha1()).str);
		return true;
	case kDbType_Sha256:
		outValue.appendChars(Sha256Str(valueSha256()).str);
		return true;
	case kDbType_Blob:
	{
		PropData blob = valueBlob();
		return base64_encode(blob.data(), blob.size(), outValue) > 0;
	}
	default:
		EASSERT(false, "unhandled string conversion in DbObjectElement::getValueSafe(StringBuilder)");
		return false;
	}
}

DbObjectElement::CompareResult DbObjectElement::compare(const DbObjectElement& rhs) const
{
	if(isNullOrEmpty())
		return rhs.isNullOrEmpty() ? Cmp_Eq : Cmp_Sm;

	if(rhs.isNullOrEmpty())
		return Cmp_Lg;

	if(type() != rhs.type())
		return Cmp_Err;

	switch(type())
	{
		// case kDbType_Array:
		// 	return valueArray().compare(rhs.valueArray());
    	// case kDbType_Object:
		// 	return valueObject().compare(rhs.valueObject());
    	case kDbType_ObjectId:
		{
			const DbOid& a = valueOid();
			const DbOid& b = rhs.valueOid();

			s32 timediff = a.id[0] - b.id[0];
			return timediff == 0 ? Cmp_Eq : (timediff < 0 ? Cmp_Sm : Cmp_Lg);			
		}
    	case kDbType_Bool:
		{
			s8 diff = (valueBool() & 1) - (rhs.valueBool() & 1);
			return diff == 0 ? Cmp_Eq : (diff < 0 ? Cmp_Sm : Cmp_Lg);
		}
    	case kDbType_String:
		{
			const char* a = valueString();
			const char* b = rhs.valueString();
			return (CompareResult) compareCase(a, b);
			//return (CompareResult) utf8_cmp(a, b);
		}
    	case kDbType_Integer:
		{
			u32 a = valueInteger();
			u32 b = rhs.valueInteger();
			u32 diff = a - b;
			return diff == 0 ? Cmp_Eq : (diff < 0 ? Cmp_Sm : Cmp_Lg);
		}
    	case kDbType_Long:
		{
			u64 a = valueLong();
			u64 b = rhs.valueLong();
			u64 diff = a - b;
			return diff == 0 ? Cmp_Eq : (diff < 0 ? Cmp_Sm : Cmp_Lg);
		}
    	case kDbType_VarInt:
		{
			u64 a = valueVarInt().value;
			u64 b = rhs.valueVarInt().value;
			u64 diff = a - b;
			return diff == 0 ? Cmp_Eq : (diff < 0 ? Cmp_Sm : Cmp_Lg);
		}
    	case kDbType_Float:
		{
			float a = valueFloat();
			float b = rhs.valueFloat();
			float diff = a - b;
			return diff == 0 ? Cmp_Eq : (diff < 0 ? Cmp_Sm : Cmp_Lg);
		}
    	case kDbType_Double:
		{
			double a = valueDouble();
			double b = rhs.valueDouble();
			double diff = a - b;
			return diff == 0 ? Cmp_Eq : (diff < 0 ? Cmp_Sm : Cmp_Lg);
		}
    	case kDbType_Timestamp:
		{
			// NOTE: DateTime strcutre includes milliseconds too!
			int diff = valueTime().compare(rhs.valueTime());
			return diff == 0 ? Cmp_Eq : (diff < 0 ? Cmp_Sm : Cmp_Lg);
		}
		case kDbType_Blob:
		{
			u32 a = valueBlob().size();
			u32 b = rhs.valueBlob().size();
			u32 diff = a - b;
			return diff == 0 ? Cmp_Eq : (diff < 0 ? Cmp_Sm : Cmp_Lg);
		}
		default:
			return Cmp_Err;
	}
}

void DbObjectElement::toPlistString(StringEditor& str, const SkeletonBuildParams& params, u32 depth) const
{
	bool indent = params.pretty && depth;

	const char* key = name();
	bool hasKey = !isnil(key);
	if(hasKey)
	{
		if(indent)
		{
			for(u32 i = 0; i < depth; ++i)
				str.appendChar('\t');
		}

		str << "<key>" << key << "</key>";
		if(params.pretty)
			str << '\n';
	}

	if(isDbVariant())
	{
		valueObject().toPlistString(str, params, depth);
		return;
	}

	if(indent && !isDbVariant())
	{
		for(u32 i = 0; i < depth; ++i)
			str.appendChar('\t');
	}

	const u8* ptr = value();
	switch(type())
	{
	case kDbType_Null:
		str << "<data></data>";
		break;
	case kDbType_Bool:
		str << ((ptr[0] > 0) ? "<true/>" : "<false/>");
		break;
	case kDbType_String:
		str << "<string>" << (const char*)ptr << "</string>";
		break;
	case kDbType_Integer:
		str << "<integer>";
		tostring(*(const u32*)ptr, str);
		str << "</integer>";
		break;
	case kDbType_Long:
		str << "<integer>";
		tostring(*(const u64*)ptr, str);
		str << "</integer>";
		break;
	case kDbType_VarInt:
		str << "<integer>";
		tostring(decodeVarUint64(ptr), str);
		str << "</integer>";
		break;
	case kDbType_Float:
		str << "<real>";
		tostring(*(float*)ptr, str);
		str << "</real>";
		break;
	case kDbType_Double:
		str << "<real>";
		tostring(*(double*)ptr, str);
		str << "</real>";
		break;
	case kDbType_Timestamp:
		str << "<date>";
		valueTime().toString(str);
		str << "</date>";
		break;
	case kDbType_Guid:
		str << "<string>";
		str.appendChars(GuidStr(*(Guid*)ptr).str);
		str << "</string>";
		break;
	case kDbType_Sha1:
		str << "<string>";
		str.appendChars(Sha1Str(*(SHA1*)ptr).str);
		str << "</string>";
	case kDbType_Sha256:
		str << "<string>";
		str.appendChars(Sha256Str(*(SHA256*)ptr).str);
		str << "</string>";
		break;
	case kDbType_Blob:
		str << "<data>";
		base64_encode(value(), objectSize(), str);
		str << "</data>";
		break;
	case kDbType_ObjectId:
		str << "<string>";
		(*(DbOid*)ptr).toString(str);
		str << "</string>";
		break;
	default:
		EASSERT(false, "Unhandled db element type!");
	}

	if(params.pretty)
		str << '\n';
}

void DbObjectElement::toJsonString(StringEditor& str, const JsonConversionParams& params, u32 depth) const
{
	bool indent = params.indentChildren && depth;
	if(params.indentRoots && depth == 1)
	{
		indent = false;
	}

	if(indent)
	{
		for(u32 i = 0; i < depth; ++i)
			str.appendChar('\t');
	}

	const char* key = name();
	if(!isnil(key))
	{
		str << '\"' << key << "\": ";
	}

	if(isDbVariant())
	{
		valueObject().toJsonString(str, params, depth);
		return;
	}

	bool needsQuote =   type() == kDbType_String ||
						type() == kDbType_Guid ||
						type() == kDbType_Sha1 ||
						type() == kDbType_Sha256 ||
						type() == kDbType_ObjectId ||
						type() == kDbType_Blob; 
	
	if(needsQuote)
		str.appendChar('\"');

	const u8* ptr = value();

	switch(type())
	{
	case kDbType_Null:
		str << "null";
		break;
	case kDbType_Bool:
		str << ((ptr[0] > 0) ? "true" : "false");
		break;
	case kDbType_String:
		str << (const char*)ptr;
		break;
	case kDbType_Integer:
		tostring(*(const u32*)ptr, str);
		break;
	case kDbType_Long:
		tostring(*(const u64*)ptr, str);
		break;
	case kDbType_VarInt:
		tostring(decodeVarUint64(ptr), str);
		break;
	case kDbType_Float:
		tostring(*(float*)ptr, str);
		break;
	case kDbType_Double:
		tostring(*(double*)ptr, str);
		break;
	case kDbType_Guid:
		str.appendChars(GuidStr(*(Guid*)ptr).str);
		break;
	case kDbType_Sha1:
		str.appendChars(Sha1Str(*(SHA1*)ptr).str);
	case kDbType_Sha256:
		str.appendChars(Sha256Str(*(SHA256*)ptr).str);
		break;
	case kDbType_Blob:
		base64_encode(value(), objectSize(), str);
		break;
	case kDbType_ObjectId:
		(*(DbOid*)ptr).toString(str);
		break;
	default:
		EASSERT(false, "Unhandled db element type!");
	}

	if(needsQuote)
		str << '\"';
}
	
DbObject::DbObject() 
	: m_data(nullptr)
	, m_ownership(DatabaseOwnershipPolicy::Assume)
{}

DbObject::DbObject(u8* data, DatabaseOwnershipPolicy ownership)
	: m_data(data)
	, m_ownership(ownership)
{}

DbObject::DbObject(const DbObject& other)
	: m_data(other.m_data)
	, m_ownership(DatabaseOwnershipPolicy::Shared)
{}

DbObject::DbObject(DbObject&& other) noexcept
	: m_data(other.m_data)
	, m_ownership(other.m_ownership)
{
	other.m_data = nullptr;
	other.m_ownership = DatabaseOwnershipPolicy::Shared;
}

DbObject::~DbObject()
{
	if(m_data)
	{
		if(m_ownership != DatabaseOwnershipPolicy::Shared)
		{
			std::memset(m_data, 0, dataSize());
			HeapFreeGlobal(m_data);
		}
		
		m_data = nullptr;
	}
}

DbObject DbObject::clone() const
{
	u8* newData = static_cast<u8*>(HeapAllocGlobal(dataSize()));
	std::memcpy(newData, data(), dataSize());
	return { newData, DatabaseOwnershipPolicy::Borrowed };
}

bool DbObject::isNull() const
{
	return m_data == nullptr;
}

bool DbObject::isNullOrEmpty() const
{
	if(!m_data)
		return true;

	return tableSizeBytes() == 0;
}

DbObjectElement
DbObject::at(int index) const
{
	DbObjectElementIterator it(*this);
	DbObjectElement el;
	
	for(int i = 0; i <= index; i++)
		el = it.next();

	if(el.isEoo())
		return { nullptr };
	
	return el;
}
	
DbObjectElement 
DbObject::queryField(const char* name) const
{
	if(isNullOrEmpty())
		return {nullptr};
	
	for(DbObjectElementIterator it(*this); it.more();)
	{
		DbObjectElement el = it.next();

		if(el && !compareIgnoreCase(el.name(), name))
		{
			return el;
		}
	}

	return nullptr;
}

DbObjectElement 
DbObject::queryField(const char* name, int nameLength) const
{
	if(isNullOrEmpty())
		return {nullptr};
	
	for(DbObjectElementIterator it(*this); it.more();)
	{
		DbObjectElement el = it.next();

		if(el && !compareIgnoreCase(el.name(), name, nameLength))
		{
			return el;
		}
	}

	return {nullptr};
}

DbObjectElement
DbObject::queryFieldDotted(const char* name) const
{
	bool isWild = false;
	
	DbObjectElement el = queryFieldDotted(name, isWild);
	if (isWild && !el.isDbVariant())
		return {nullptr};
	else
		return el;
}

DbObjectElement
DbObject::queryFieldDotted(const char* name, bool& isWild) const
{
	const char* dot = strchr(name, '.');

	if (!dot)
		return queryField(name);

	DbObjectElement e = queryField(name, int(dot - name));

	if (e.isEoo())
		return e;

	DbObject o = e.valueObject();

	if (o.isNullOrEmpty())
		return {nullptr};

	if (dot[1] == '*')
	{
		isWild = true;
		return e;
	}

	return o.queryFieldDotted(dot + 1, isWild);
}
	
u8* 
DbObject::tableData() const
{
	if(!m_data)
	{
		return nullptr;
	}

	u8* ptr = m_data;

	if ((*ptr++ & kDbType_Anonymous) == 0)
	{
		while (*ptr++)
			;
	}

	ptr += decodeSize(ptr);
	return ptr;
}
u32 
DbObject::tableSizeBytes() const
{
	const u8* ptr = m_data;

	if ((*ptr++ & kDbType_Anonymous) == 0)
	{
		while (*ptr++)
			;
	}

	return decodeVarUint(ptr);
}

u32 DbObject::fieldCount() const
{
	u32 count = 0;

	for(DbObjectElementIterator it(*this); it.more();)
	{
		it.next();
		++count;
	}

	return count;
}

void DbObject::toPlistString(StringEditor& str, const SkeletonBuildParams& params, u32 depth) const
{
	bool indent = params.pretty && depth;
	
	if(!depth)
	{
		str << "<plist version=\"1.0\">";
		if(params.pretty)
			str << '\n';
	}

	if(indent)
	{
		for(u32 i = 0; i < depth; ++i)
			str.appendChar('\t');
	}

	str << (isArray() ? "<array>" : "<dict>");
	if(params.pretty)
		str << '\n';
	
	++depth;
	for(DbObjectElementIterator it(*this); it.more();)
	{
		DbObjectElement e = it.next();
		e.toPlistString(str, params, depth);
	}
	--depth;

	if(indent)
	{
		for(u32 i = 0; i < depth; ++i)
			str.appendChar('\t');
	}

	str << (isArray() ? "</array>" : "</dict>");
	if(params.pretty)
		str << '\n';

	if(!depth)
		str << "</plist>";
}

bool DbObject::fromPlistString(const char* str)
{
	return internalFromPlistString(str, strlen(str), false);
}

bool DbObject::fromPlistString(const char* str, size_t strLen)
{
	return internalFromPlistString(str, strLen, false);
}

bool DbObject::fromPlistString(StringEditor& str)
{
	return internalFromPlistString(str.c_str(), str.size(), false);
}


void DbObject::toJsonString(StringEditor& str, const JsonConversionParams& params, u32 depth) const
{
	if(isArray())
		str << "[";
	else
		str << "{";

	bool indentRoots = params.indentRoots && !depth;
	bool indentChildren = params.indentChildren && depth;
	bool indent = indentRoots || indentChildren;

	if(indent)
	{
		str << '\n';
	}

	{
		DbObjectElementIterator it(*this);
		DbObjectElement e = it.next();
		while(!e.isEoo())
		{
			if(indentRoots)
				str << "\t";

			e.toJsonString(str, params, depth + 1);
			e = it.next();
		
			if(!e.isEoo())
			{
				str << ", ";
				if(indent)
					str << '\n';
			}
		}
	}

	if(indentChildren)
	{
		str << '\n';

		for(u32 i = 0; i < depth; i++)
			str << '\t';
	}

	if(indentRoots)
		str << '\n';

	if(isArray())
		str << "]";
	else
		str << "}";
}

bool PLIST_toPrototypeObject(DbObjectBuilder& builder, XmlDocumentNode* tag, bool isRootDictionary)
{
	while(tag)
	{
		const char* key = NO;
		if(equalsIgnoreCase("key", tag->getNodeName()))
		{
			key = tag->getText();
			tag = tag->getSibling();
		}

		const char* valueType = tag->getNodeName();
		const char* valueString = tag->getText();

		if(equalsIgnoreCase("string", valueType))
		{
			builder << key << valueString;
		}
		else if(equalsIgnoreCase("data", valueType))
		{
			StringBuilder<> sb;
			if(!base64_decode(valueString, strlen(valueString), sb))
				return false;

			builder << key << PropData(sb.detach(), sb.size(), DatabaseOwnershipPolicy::Assume);
		}
		else if(equalsIgnoreCase("date", valueType))
		{
			DateTime date;
			if(!date.fromString(valueString))
				return false;

			builder << key << date;
		}
		else if(equalsIgnoreCase("integer", valueType))
		{
			u64 integer;
			if(!tointeger(valueString, integer))
				return false;

			u32 smallInteger = static_cast<u32>(integer);

			if(smallInteger == integer)
				builder << key << smallInteger;
			else
				builder << key << integer;
		}
		else if(equalsIgnoreCase("real", valueType))
		{
			double real = atof(valueString);
			float smallReal = static_cast<float>(real);
			if(smallReal == real)
				builder << key << smallReal;
			else
				builder << key << real;
		}
		else if(equalsIgnoreCase("true", valueType))
		{
			builder << key << true;
		}
		else if(equalsIgnoreCase("false", valueType))
		{
			builder << key << false;
		}
		else if(equalsIgnoreCase("array", valueType))
		{
			DbObjectArrayBuilder array;
			if(!PLIST_toPrototypeObject(array, tag->getChild(), NO))
				return false;

			builder << key << array.done();
		}
		else if(equalsIgnoreCase("dict", valueType))
		{
			DbObjectBuilder dict;
			if(!PLIST_toPrototypeObject(dict, tag->getChild(), NO))
				return false;

			if(isRootDictionary)
			{
				builder = std::move(dict);
				return YES;
			}
			else
			{
				builder << key << dict.done();
			}
		}

		tag = tag->getSibling();
	}

	return true;
}

bool DbObject::internalFromPlistString(const char* str, size_t strLen, bool array)
{
	if(isnil(str))
		return false;

	MemorySharedBuffer sb(str, strLen);
	XmlDocument* doc = XML_Parse(&sb);

	XmlDocumentNode* rootTag = doc->getNode();
	if(!equalsIgnoreCase("plist", rootTag->getNodeName()))
		return false;

	rootTag = rootTag->getChild();
	if(!equalsIgnoreCase("dict", rootTag->getNodeName()))
		return false;

	DbObjectBuilder builder;
	if(!PLIST_toPrototypeObject(builder, rootTag, YES))
		return false;

	DbObject dbo = builder.done();
	m_data = dbo.m_data; dbo.m_data = nullptr;
	m_ownership = dbo.m_ownership; dbo.m_ownership = DatabaseOwnershipPolicy::Shared;
	delete doc;

	return true;
}

bool DbObject::internalFromJsonString(const char* str, size_t strLen, bool array)
{
	if(isnil(str))
		return false;

	JsonParser parser(str, static_cast<u32>(strLen));
	json_object* header_root = parser.parse();
	if(header_root == nullptr)
		return false;

	if(array && header_root->type != json_object::eTypeArray)
		return false;

	DbObject dbo;
	if(array) {
		DbObjectArrayBuilder builder;
		JSON_toPrototypeArray(header_root, builder);
		dbo = builder.done();
	}
	else {
		DbObjectBuilder builder;
		JSON_toPrototypeObject(header_root, builder);
		dbo = builder.done();
	}

	m_data = dbo.m_data; dbo.m_data = nullptr;
	m_ownership = dbo.m_ownership; dbo.m_ownership = DatabaseOwnershipPolicy::Shared;
	return true;
}
	
bool DbObject::fromJsonString(const char* str)
{
	return internalFromJsonString(str, rw::strlen(str), false);
}

bool DbObject::fromJsonString(const char* str, size_t strLen)
{
	return internalFromJsonString(str, strLen, false);
}

bool DbObject::fromJsonString(StringEditor& str)
{
	return internalFromJsonString(str.c_str(), str.size(), false);
}

bool DbObject::operator==(const DbObject& rhs) const
{
	return m_data == rhs.m_data;
}

DbObjectElement DbObject::operator[](const char* key) const
{
	return queryFieldDotted(key);
}

DbObjectElement DbObject::operator[](int index) const
{
	return at(index);
}
	
DbObject& DbObject::operator=(const DbObject& rhs) noexcept
{
	m_data = rhs.m_data;
	m_ownership = DatabaseOwnershipPolicy::Shared;
	return *this;
}

DbObject& DbObject::operator=(DbObject&& rhs) noexcept
{
	m_data = rhs.m_data;
	m_ownership = rhs.m_ownership;
	rhs.m_data = nullptr;
	rhs.m_ownership = DatabaseOwnershipPolicy::Shared;
	return *this;
}

DbObjectArray::DbObjectArray()
	: DbObject()
{}

DbObjectArray::DbObjectArray(u8* data, DatabaseOwnershipPolicy ownership)
	: DbObject(data, ownership)
{}

DbObjectArray::DbObjectArray(const DbObjectArray& other)
	: DbObject(other)
{}
	
DbObjectArray::DbObjectArray(DbObjectArray&& other) noexcept
	: DbObject(other)
{}

bool DbObjectArray::fromJsonString(const char* str)
{
	return internalFromJsonString(str, rw::strlen(str), true);
}

bool DbObjectArray::fromJsonString(const char* str, size_t strLen)
{
	return internalFromJsonString(str, strLen, true);
}

bool DbObjectArray::fromJsonString(StringEditor& str)
{
	return internalFromJsonString(str.c_str(), str.size(), true);
}

DbObjectElement DbObjectArray::operator[](int index) const
{
	return DbObject::operator[](index);
}

DbObjectArray& DbObjectArray::operator=(const DbObjectArray& rhs) noexcept
{
	DbObject::operator=(rhs);
	return *this;
}

DbObjectArray& DbObjectArray::operator=(DbObjectArray&& rhs) noexcept
{
	DbObject::operator=(rhs);
	return *this;
}
}
