#pragma once
#include "TypeInfo.h"
#include <Engine.Base/Vec.h>
#include "ArrayInfoMacros.h"

namespace rw
{
typedef void(*SetElementCountFun_t)(void*, uint);
typedef void(*ReleaseElementsFun_t)(void*);

class ArrayTypeInfo;

struct ArrayTypeInfoData
{
    TypeInfoData basic;
    const TypeInfo* elementType;
    SetElementCountFun_t setElementCount;
    ReleaseElementsFun_t releaseElements;
};

class ArrayTypeInfo : public TypeInfo
{
public:
    ArrayTypeInfo(const ArrayTypeInfoData* infoData)
        : TypeInfo(const_cast<TypeInfoData*>(&infoData->basic))
    {}
    virtual ~ArrayTypeInfo() = default;

    ImplementInfoDataGetter(ArrayTypeInfoData)

    const TypeInfo* getElementType() const
        { return getArrayTypeInfoData()->elementType; }

    FB_REFL_API void setElementCount(void* placement, uint newCount);
    FB_REFL_API void releaseElements(void* placement);

    FB_REFL_API SkeletonNode* toSkel(const void* data, const SkelConversionContext* context) const override;
    FB_REFL_API void toString(const void* data, const StringConversionParams* params, StringEditor& result) const override;
};
}