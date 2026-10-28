#include <Engine.Reflection/TypeInfo.h>
#include <Engine.Reflection/PrimitiveTypeInfo.h>
#include <Engine.Reflection/ArrayBase.h>
#include <Engine.Reflection/ArrayTypeInfo.h>
#include <Engine.Reflection/EnumTypeInfo.h>
#include <Engine.Reflection/ValueTypeInfo.h>
#include <Engine.Reflection/ClassInfo.h>
#include <Engine.Base/Crypto/SHA.h>
#include <Engine.Base/Crypto/SHAFunctions.h>

namespace rw
{
TypeInfo::TypeInfo(TypeInfoData* infoData)
    : MemberInfo(&infoData->member)
{
    m_nextTypeInfo =  s_firstTypeInfo;
    s_firstTypeInfo = this;
}

TypeInfo::~TypeInfo()
{
    m_nextTypeInfo = nullptr;
}

const FieldInfoData* TypeInfo::getField(const char* name) const
{
    const FieldInfoData* fieldInfoDatas = getTypeInfoData()->fieldInfos;
    uint count = getTypeInfoData()->fieldCount;

    for(uint i = 0; i < count; ++i)
    {
        const FieldInfoData& fieldInfo = fieldInfoDatas[i];
        if(equalsIgnoreCase(fieldInfo.member.name, name))
            return &fieldInfo;
    }

    return nullptr;
}

const FieldInfoData* TypeInfo::getFieldAtIndex(uint index) const
{
    const FieldInfoData* fieldInfoDatas = getTypeInfoData()->fieldInfos;
    uint count = getTypeInfoData()->fieldCount;
    return (index < count) ? fieldInfoDatas + index : nullptr;
}

const FieldInfoData* TypeInfo::getFieldAtOffset(uint offset) const
{
    const FieldInfoData* fieldInfoDatas = getTypeInfoData()->fieldInfos;
    uint count = getTypeInfoData()->fieldCount;

    for(uint i = 0; i < count; ++i)
    {
        const FieldInfoData& fieldInfo = fieldInfoDatas[i];
        if(fieldInfo.fieldOffset == offset)
            return &fieldInfo;
    }

    return nullptr;
}

const char* MemberInfo::getAttribute(const char* name) const
{
    u16 count = m_infoData->attributeCount;
    const AttributeSpec* attrSpecs = m_infoData->attributes;
    for(u16 i = 0; i < count; i++)
    {
        if(equalsIgnoreCase(attrSpecs[i].name, name))
            return attrSpecs[i].value;
    }

    return nullptr;
}

void MemberInfo::createMember(void* placement) const
{
    switch(getTypeCategory())
    {
    case kTypeCategory_Class:
    {
        ClassInfo* classInfo = (ClassInfo*)this;
        classInfo->createRef(placement);
        break;
    }
    case kTypeCategory_Array:
    {
        *(void**)placement = &array_helper::s_emptyArrayData[1];
        break;
    }
    case kTypeCategory_Enum:
    {
        *(int*)placement = ((EnumTypeInfo*)this)->getDefaultValue();
        break;
    }
    case kTypeCategory_ValueType:
    {
        ValueTypeInfo* structInfo = (ValueTypeInfo*)this;
        const FieldInfoData* fields = structInfo->getFieldInfos();
        u32 fieldCount = structInfo->getFieldCount();

        for(u32 i = 0; i < fieldCount; i++)
        {
            const FieldInfoData& fid = fields[i];
            fid.fieldType->createMember((u8*)placement + fid.fieldOffset);
        }
        break;
    }
    case kTypeCategory_Member:
    {
        PrimitiveTypeInfo* primInfo = (PrimitiveTypeInfo*)this;
        std::memcpy(placement, primInfo->getDefaultValue(), primInfo->getMemberSize());
        break;
    }
    default:
        break;
    }
}

void MemberInfo::copyCreateMember(void* placement, void* source) const
{
    switch(getTypeCategory())
    {
    case kTypeCategory_Class:
    {
        ClassInfo* classInfo = (ClassInfo*)this;
        classInfo->destroyRef(placement);
        classInfo->setRef(placement, source);
        break;
    }
    case kTypeCategory_Array:
    {
        //ArrayTypeInfo* arrayInfo = (ArrayTypeInfo*)this;
        break;
    }
    case kTypeCategory_Member:
    {
        PrimitiveTypeInfo* primInfo = (PrimitiveTypeInfo*)this;
        std::memcpy(placement, source, primInfo->getMemberSize());
        break;
    }
    default:
        break;
    }
}

void MemberInfo::destroyMember(void* placement) const
{
    switch(getTypeCategory())
    {
    case kTypeCategory_Class:
    {
        ClassInfo* classInfo = (ClassInfo*)this;
        classInfo->destroyRef(placement);
        break;
    }
    case kTypeCategory_Array:
    {
        //ArrayTypeInfo* arrayInfo = (ArrayTypeInfo*)this;
        break;
    }
    case kTypeCategory_Member:
    {
        PrimitiveTypeInfo* primInfo = (PrimitiveTypeInfo*)this;
        std::memset(placement, 0, primInfo->getMemberSize());
        break;
    }
    default:
        break;
    }
}

Guid genTypeGuidInternal(const char* moduleName, const char* typeName, const char* seed)
{
    Crypto::SHA1Context context;
    Crypto::SHA2::SHA1Initialize(context, NO, 0);
    Crypto::SHA2::SHAUpdate(context, moduleName, strlen(moduleName));
    Crypto::SHA2::SHAUpdate(context, typeName, strlen(typeName));
    Crypto::SHA2::SHAUpdate(context, seed, strlen(seed));

    u8 result[Crypto::SHA1Context::kHashSize];
    Crypto::SHA2::SHA1Finalize(context, result);

    Guid typeGuid;
    std::memcpy(&typeGuid, &result, sizeof(typeGuid));
    return typeGuid;
}

Guid genTypeGuid(const char* moduleName, const char* typeName)
{
    constexpr const char* c_typeSeed = "_typeID_0";
    return genTypeGuidInternal(moduleName, typeName, c_typeSeed);
}

Guid genArrayTypeGuid(const char* moduleName, const char* typeName)
{
    constexpr const char* c_typeSeed = "_typeID_1";
    return genTypeGuidInternal(moduleName, typeName, c_typeSeed);
}
}