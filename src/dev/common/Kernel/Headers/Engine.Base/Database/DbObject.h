#pragma once
#include "DbData.h"
#include "DbOid.h"
#include "../Misc/Xml.h"
#include "../Misc/Time.h"
#include "../String/String.h"
#include "../Crypto/SHA.h"
#include "../Crypto/Guid.h"

namespace rw
{
enum DbTypesEnum
{
    kDbType_Eoo = 0,				///< Object terminator
    kDbType_Array = 1,				///< Variable-length array, encoded just like an object
    kDbType_Object = 2,				///< Inline DbObject
    kDbType_HomoArray = 3,			///< Homogeneous array - no type or "name" tags per element
    kDbType_Null = 4,				///< Null value, no payload
    kDbType_ObjectId = 5,			///< Unique object identifier
    kDbType_Bool = 6,				///< Single-byte boolean
    kDbType_String = 7,				///< Null-terminated string
    kDbType_Integer = 8,			///< 32-bit integer
    kDbType_Long = 9,				///< 64-bit integer
    kDbType_VarInt = 10,			///< Variable-length encoded integer
    kDbType_Float = 11,				///< 32-bit floating point
    kDbType_Double = 12,			///< 64-bit floating point
    kDbType_Timestamp = 13,			///< 64-bit timestamp (Windows FILETIME)

    kDbType_Guid = 15,				///< 128-bit GUID, or MD5 hash
    kDbType_Sha1 = 16,				///< 160-bit Sha1 hash
    kDbType_Matrix44 = 17,			///< 4x4 float matrix
    kDbType_Vector4 = 18,			///< 4 float vector
    kDbType_Blob = 19,				///< Inline variable-length binary blob
    kDbType_Attachment = 20,		///< Encoded as Sha1 reference to attachment payload
    kDbType_Timespan = 21,			///< Time span (duration) in microseconds encoded as variable-length integer
    kDbType_Sha256 = 22,

    kDbType_Mask = 31,
    kDbType_Anonymous = 0x80,		/**< When set, element name is simply omitted */
};
FB_BASE_API const char* tostring(DbTypesEnum type);

struct DbObjectScatter
{
    u8* data;
    u32 dataSize;
};

struct CVarInt32 { u32 value; inline CVarInt32(u32 n) : value(n) {} };
struct CVarInt64 { u64 value; inline CVarInt64(u64 n) : value(n) {} };
struct CNull {};

#ifdef SYSARCH_X86
typedef CVarInt32 CVarInt;
#else
typedef CVarInt64 CVarInt;
#endif

class DbObject;
class DbObjectArray;

struct JsonConversionParams
{
    bool indentRoots;
    bool indentChildren;
};

// Represents a field/array element in a DbObject
class DbObjectElement
{
public:
    inline DbObjectElement()
        : m_payload(nullptr)
    {}

    inline DbObjectElement(u8* payload)
        : m_payload(payload)
    {}

    friend class any;
    friend class DbObject;

    FB_BASE_API const char* name() const;
    FB_BASE_API DbTypesEnum type() const;

    //size of element data
    FB_BASE_API u32 size() const;
    //size of (object, array, string, blob)
    FB_BASE_API u32 objectSize() const;

    inline bool isEoo()                const { return type() == kDbType_Eoo; }
    FB_BASE_API bool isNull()          const;
    FB_BASE_API bool isNullOrEmpty()   const;
    inline bool isAnonymous()          const { return !!(m_payload[0] & kDbType_Anonymous); }
    inline bool isObject()             const { return type() == kDbType_Object; }
    inline bool isArray()              const { return type() == kDbType_Array; }
    inline bool isString()             const { return type() == kDbType_String; }
    inline bool isBlob()               const { return type() == kDbType_Blob; }
    inline bool isDbVariant()          const { return isObject() || isArray(); }

    //raw element data
    FB_BASE_API u8*                     payload() const;
    //element value pointer
    FB_BASE_API u8*                     value() const;
    FB_BASE_API bool                    valueBool(bool defaultValue = false) const;
    FB_BASE_API const char*             valueString(const char* defaultValue = nullptr) const;
    FB_BASE_API u32                     valueInteger(u32 defaultValue = 0) const;
    FB_BASE_API u64                     valueLong(u64 defaultValue = 0) const;
    FB_BASE_API CVarInt                 valueVarInt(CVarInt defaultValue = 0) const;
    FB_BASE_API f32                     valueFloat(f32 defaultValue = 0.0f) const;
    FB_BASE_API f64                     valueDouble(f64 defaultValue = 0.0f) const;
    FB_BASE_API PropData                valueBlob(PropData defaultValue = c_emptyDbData) const;
    FB_BASE_API const DbOid&            valueOid(const DbOid& defaultValue = DbOid::c_zero) const;
    FB_BASE_API DbObject                valueObject() const;
    FB_BASE_API DbObjectArray           valueArray() const;
    FB_BASE_API const Guid&             valueGuid(const Guid& defaultValue = Guid()) const;
    FB_BASE_API const SHA1&             valueSha1(const SHA1& defaultValue = SHA1()) const;
    FB_BASE_API const SHA256&           valueSha256(const SHA256& defaultValue = SHA256()) const;
    FB_BASE_API const DateTime&         valueTime(const DateTime& defaultValue = DateTime()) const;

    FB_BASE_API bool                    getValueSafe(bool &outValue) const;
    FB_BASE_API bool                    getValueSafe(s32 &outValue) const;
    FB_BASE_API bool                    getValueSafe(u32 &outValue) const;
    FB_BASE_API bool                    getValueSafe(s64 &outValue) const;
    FB_BASE_API bool                    getValueSafe(u64 &outValue) const;
    FB_BASE_API bool                    getValueSafe(float &outValue) const;
    FB_BASE_API bool                    getValueSafe(double &outValue) const;

    enum CompareResult
    {
        Cmp_Sm = -1,    // smaller than
        Cmp_Lg = 1,     // greater than
        Cmp_Eq = 0,     // equals
        Cmp_Err = 128   // error
    };

    // Compare the two types if it's comparable.
    FB_BASE_API CompareResult           compare(const DbObjectElement& rhs) const;

    // Converts any reasonable type to a string.
    FB_BASE_API bool                    getValueSafe(StringEditor& outValue) const;
    
    FB_BASE_API void                    toPlistString(StringEditor& str, const SkeletonBuildParams& params, u32 depth) const;
    FB_BASE_API void                    toJsonString(StringEditor& str, const JsonConversionParams& params, u32 depth) const;

    inline operator bool()              const { return !isNullOrEmpty(); }

private:
    u8* m_payload;
};

class DbObject
{
public:
    FB_BASE_API DbObject();
    FB_BASE_API DbObject(u8* data, DatabaseOwnershipPolicy ownership = DatabaseOwnershipPolicy::Assume);
    FB_BASE_API DbObject(const DbObject& other);
    FB_BASE_API DbObject(DbObject&& other) noexcept;
    FB_BASE_API ~DbObject();

    FB_BASE_API DbObject                clone() const;
    
    DatabaseOwnershipPolicy             getOwnership() const { return m_ownership; }

    bool                                isShared() const { return m_ownership == DatabaseOwnershipPolicy::Shared; }
    bool                                isArray() const { return DbTypesEnum(m_data[0] & kDbType_Mask) == kDbType_Array; }
    FB_BASE_API bool                    isNull() const;
    FB_BASE_API bool                    isNullOrEmpty() const;

    //pointer to raw data
    u8*                                 data() const { return m_data; }    
    //size of raw data
    u32                                 dataSize() const { return u32((tableData() + tableSizeBytes()) - data()); }

    //pointer to elements data
    FB_BASE_API u8*                     tableData() const;
    //size of elements data
    FB_BASE_API u32                     tableSizeBytes() const;

    FB_BASE_API DbObjectElement         at(int index) const;
    FB_BASE_API DbObjectElement         queryField(const char* name) const;
    FB_BASE_API DbObjectElement         queryField(const char* name, int nameLength) const;
    FB_BASE_API DbObjectElement         queryFieldDotted(const char* name) const;
    FB_BASE_API DbObjectElement         queryFieldDotted(const char* name, bool& isWild) const;
    FB_BASE_API u32                     fieldCount() const;

    FB_BASE_API void                    toPlistString(StringEditor& str, const SkeletonBuildParams& params, u32 depth = 0) const;
    FB_BASE_API bool                    fromPlistString(const char* str);
    FB_BASE_API bool                    fromPlistString(const char* str, size_t strLen);
    FB_BASE_API bool                    fromPlistString(StringEditor& str);

    FB_BASE_API void                    toJsonString(StringEditor& str, const JsonConversionParams& params, u32 depth = 0) const;
    FB_BASE_API bool                    fromJsonString(const char* str);
    FB_BASE_API bool                    fromJsonString(const char* str, size_t strLen);
    FB_BASE_API bool                    fromJsonString(StringEditor& str);

    FB_BASE_API bool operator==(const DbObject& rhs) const;
    FB_BASE_API DbObjectElement operator[](const char* key) const;
    FB_BASE_API DbObjectElement operator[](int index) const;
    FB_BASE_API DbObject& operator=(const DbObject& rhs) noexcept;
    FB_BASE_API DbObject& operator=(DbObject&& rhs) noexcept;

protected:
    FB_BASE_API bool internalFromPlistString(const char* str, size_t strLen, bool array);
    FB_BASE_API bool internalFromJsonString(const char* str, size_t strLen, bool array);
    
private:
    u8* m_data;
    DatabaseOwnershipPolicy m_ownership;
};

inline const DbObject c_NullObject = DbObject();

class DbObjectArray : public DbObject
{
public:
    FB_BASE_API DbObjectArray();
    FB_BASE_API DbObjectArray(u8* data, DatabaseOwnershipPolicy ownership = DatabaseOwnershipPolicy::Assume);
    FB_BASE_API DbObjectArray(const DbObjectArray& other);
    FB_BASE_API DbObjectArray(DbObjectArray&& other) noexcept;
    
    FB_BASE_API bool                    fromJsonString(const char* str);
    FB_BASE_API bool                    fromJsonString(const char* str, size_t strLen);
    FB_BASE_API bool                    fromJsonString(StringEditor& str);

    FB_BASE_API DbObjectElement operator[](int index) const;
    FB_BASE_API DbObjectArray& operator=(const DbObjectArray& rhs) noexcept;
    FB_BASE_API DbObjectArray& operator=(DbObjectArray&& rhs) noexcept;
};

class DbObjectElementIterator
{
public:
    inline DbObjectElementIterator(const DbObject& o)
    {
        m_current = o.tableData();
        m_end = o.data() + o.dataSize();
    }

    inline bool more() const 
    { 
        return (m_current < m_end) && (m_current[0] != kDbType_Eoo); 
    }

    inline DbObjectElement next() 
    {
        DbObjectElement elem(m_current);       
        u8* next = elem.value() + elem.size();
        m_current = m_end > next ? next : m_end;

        return elem;
    }

private:
    u8* m_current;
    u8* m_end;
};
}
