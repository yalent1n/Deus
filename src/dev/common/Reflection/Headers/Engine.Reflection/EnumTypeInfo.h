#pragma once
#include "TypeInfo.h"
#include "EnumInfoMacros.h"

namespace rw
{
struct EnumTypeInfoData
{
    TypeInfoData basic;
    int defaultValue;
};

class EnumTypeInfo : public TypeInfo
{
public:
    EnumTypeInfo(const EnumTypeInfoData* infoData)
        : TypeInfo(const_cast<TypeInfoData*>(&infoData->basic))
    {}
    ~EnumTypeInfo() = default;

    ImplementInfoDataGetter(EnumTypeInfoData)

    int getDefaultValue() const
        { return getEnumTypeInfoData()->defaultValue; }

    const void* getDefaultInstance() const override
        { return &getEnumTypeInfoData()->defaultValue; }

    
};
}