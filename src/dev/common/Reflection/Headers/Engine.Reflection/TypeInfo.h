#pragma once
#include "Bundle.h"
#include <Engine.Base/Crypto/Guid.h>
#include <Engine.Base/String/String.h>
#include <Engine.Base/String/CString.h>
#include <Engine.Base/Misc/Xml.h>

namespace rw
{
enum TypeCodeEnum : u16
{
    kTypeCode_Void,
    kTypeCode_Bool,
    kTypeCode_Int8,
    kTypeCode_Uint8,
    kTypeCode_Int16,
    kTypeCode_Uint16,
    kTypeCode_Int32,
    kTypeCode_Uint32,
    kTypeCode_Int64,
    kTypeCode_Uint64,
    kTypeCode_Float32,
    kTypeCode_Float64,
    kTypeCode_Guid,
    kTypeCode_Sha1,
    kTypeCode_CString,
    kTypeCodeCount,

    kTypeCodeBits = 4
};

enum TypeCategoryEnum : u16
{
    kTypeCategory_Member,
    kTypeCategory_Class,
    kTypeCategory_ValueType,
    kTypeCategory_Enum,
    kTypeCategory_Interface,
    kTypeCategory_Array,
    kTypeCategoryCount,

    kTypeCategoryBits = 3
};

struct MemberInfoFlags
{
    // these keywords has to match DDF keywords 
    enum : u16
    {
        NONE,
        STATIC,
        ABSTRACT,
        META,
        PUBLIC,
        PRIVATE
    };

    MemberInfoFlags(TypeCodeEnum code = kTypeCode_Void, TypeCategoryEnum category = kTypeCategory_Member, u16 flags = NONE)
        : typeCode(code)
        , typeCategory(category)
        , basicFlags(flags)
    {}

    TypeCodeEnum typeCode : kTypeCodeBits;
    TypeCategoryEnum typeCategory : kTypeCategoryBits;
    u16 basicFlags : 16 - (kTypeCodeBits + kTypeCategoryBits);
};

struct AttributeSpec
{
    const char* name;
    const char* value;
};

typedef void(*CtorFun_t)(void* placement);
typedef void(*CopyCtorFun_t)(void* placementFrom, void* placementTo);
typedef void(*DtorFun_t)(void* placement);

// Default type for root classes
class TypeInfo;

struct MemberInfoData
{
    const char* name;
    const AttributeSpec* attributes;
    u32 nameHash;
    u16 size; // NOTE: size as member
    u16 alignment; // NOTE: alignment as member
    MemberInfoFlags flags;
    u16 attributeCount;
};

interface MemberInfo
{
protected:
    MemberInfoData* m_infoData;

    MemberInfo(MemberInfoData* infoData)
        : m_infoData(infoData)
    {}

public:
const MemberInfoData* getMemberInfoData() const { return m_infoData; }
    MemberInfoData* getMemberInfoData() { return m_infoData; }

    const char* getName() const
        { return m_infoData->name; }
    
    u32 getNameHash() const
        { return m_infoData->nameHash; }

    u16 getMemberSize() const
        { return m_infoData->size; }
    
    u16 getMemberAlignment() const
        { return m_infoData->alignment; }

    TypeCodeEnum getTypeCode() const
        { return m_infoData->flags.typeCode; }
    
    TypeCategoryEnum getTypeCategory() const
        { return m_infoData->flags.typeCategory; }
    
    u16 getMemberFlags() const
        { return m_infoData->flags.basicFlags; }

    const AttributeSpec* getAttributes() const
        { return m_infoData->attributes; }
    
    u16 getAttributeCount() const
        { return m_infoData->attributeCount; }

    FB_REFL_API const char* getAttribute(const char* name) const;

    void createMember(void* placement) const;
    void copyCreateMember(void* placement, void* source) const;
    void destroyMember(void* placement) const;
};

struct FieldInfoData
{
    MemberInfoData member;
    const TypeInfo* fieldType;
    const TypeInfo* declaringType;
    u32 fieldOffset;
};

struct TypeInfoData
{
    MemberInfoData member;
    Guid guid;
    const ModuleInfo* module;
    const FieldInfoData* fieldInfos;
    uint fieldCount;
    u32 size; // NOTE: instance size
    u16 alignment; // NOTE: instance alignment
    CtorFun_t ctorFun;
    CopyCtorFun_t copyCtorFun;
    DtorFun_t dtorFun;
};

// Acts as the first type in the inheritance chain (for every root class) 
class Object
{
    // here may be the root class types
    friend class SymbolClassInfo;

public:
    virtual ~Object() = default;

    virtual const TypeInfo* getType() const
        { return m_typeInfo; }

protected:
    const TypeInfo*     m_typeInfo;
};

FB_REFL_API Guid genTypeGuid(const char* moduleName, const char* typeName);
FB_REFL_API Guid genArrayTypeGuid(const char* moduleName, const char* typeName);

#define ImplementInfoDataGetter(idt) \
    const idt* get##idt() const { return (const idt*) m_infoData; } \
    idt* get##idt() { return (idt*) m_infoData; }

class TypeInfo : public MemberInfo
{
    friend class NativeTypeRegistry;

public:
    FB_REFL_API          TypeInfo(TypeInfoData* infoData);
    FB_REFL_API virtual ~TypeInfo();

    ImplementInfoDataGetter(TypeInfoData)

    u32 getRuntimeId() const
        { return m_runtimeId; }

    u32 getInstanceSize() const
        { return getTypeInfoData()->size; }
    
    u32 getInstanceAlignment() const
        { return getTypeInfoData()->alignment; }

    const Guid& getGuid() const
        { return getTypeInfoData()->guid; }

    const ModuleInfo* getModule() const
        { return getTypeInfoData()->module; }

    const FieldInfoData* getFieldInfos() const
        { return getTypeInfoData()->fieldInfos; }
    
    uint getFieldCount() const
        { return getTypeInfoData()->fieldCount; }

    FB_REFL_API const FieldInfoData* getField(const char* name) const;
    FB_REFL_API const FieldInfoData* getFieldAtIndex(uint index) const;
    FB_REFL_API const FieldInfoData* getFieldAtOffset(uint offset) const;

    nodefine const void* getDefaultInstance() const
        { return nullptr; }

    struct SkelConversionContext
    {
        Skeleton* skel;
        SkeletonNode* root;
        bool asMember;
        bool asArray;
    };

    virtual SkeletonNode* toSkel(const void* data, const SkelConversionContext* context) const
        { return nullptr; }
    
    struct StringConversionParams
    {
        bool asMember;
    };

    virtual void toString(const void* data, const StringConversionParams* params, StringEditor& result) const
        { }

private:
    TypeInfo* m_nextTypeInfo;
    u32 m_runtimeId;
    static inline TypeInfo* s_firstTypeInfo = nullptr;
    static inline TypeInfo* s_lastTypeInfo = nullptr;
};

template<typename T> struct _TypeConv
{
	typedef T type;
};
	
template<typename T> struct _TypeConv<T*>
{
	typedef typename _TypeConv<T>::type type;
};

template<typename T> struct _TypeConv<T&>
{
	typedef typename _TypeConv<T>::type type;
};
	
template<typename T> struct _TypeConv<SmartHandle<T>>
{
	typedef typename _TypeConv<T>::type type;
};

template<typename T>
class _TypeHelper : public T
{
private:
    _TypeHelper();
    virtual ~_TypeHelper() = default;
};

template<typename T>
const auto Type()
    { return &_TypeHelper< typename _TypeConv<T>::type >::typedata::c_typeInfo; }

template<typename T>
const auto Type(const T&)
    { return &_TypeHelper< typename _TypeConv<T>::type >::typedata::c_typeInfo; }

template<typename T>
const auto Type(const T*)
    { return &_TypeHelper< typename _TypeConv<T>::type >::typedata::c_typeInfo; }

template<typename T>
const auto Type(const SmartHandle<T>&)
    { return &_TypeHelper<T>::typedata::c_typeInfo; }

template<typename T>
const auto Type(const Array<T>&)
    { return &_TypeHelper<T>::typedata::c_arrayTypeInfo; }

template<typename T>
const auto Type(const ArrayReader<T>&)
    { return &_TypeHelper<T>::typedata::c_arrayTypeInfo; }

template<typename T>
const auto Type(const ArrayEditor<T>&)
    { return &_TypeHelper<T>::typedata::c_arrayTypeInfo; }
}