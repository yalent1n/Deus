#pragma once
#include "TypeInfo.h"
#include "ValueTypeInfoMacros.h"

namespace rw
{
struct ValueTypeInfoData
{
    TypeInfoData basic;
    void* defaultInstance;
};

class ValueTypeInfo : public TypeInfo
{
public:
    ValueTypeInfo(const ValueTypeInfoData* infoData)
        : TypeInfo(const_cast<TypeInfoData*>(&infoData->basic))
    {}

    ImplementInfoDataGetter(ValueTypeInfoData)

    const void* getDefaultInstance() const override
        { return getValueTypeInfoData()->defaultInstance; }

    void* createInstance() const
        { return nullptr; }

    SkeletonNode* toSkel(const void* data, const SkelConversionContext* context) const override;
    void toString(const void* data, const StringConversionParams* params, StringEditor& result) const override;
};
}