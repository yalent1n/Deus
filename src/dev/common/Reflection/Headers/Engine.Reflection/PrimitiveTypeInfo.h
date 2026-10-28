#pragma once
#include "ArrayTypeInfo.h"
#include "TypeInfoMacros.h"
#include <Engine.Base/String/CString.h>
#include <Engine.Base/Vec.h>

namespace rw
{
class PrimitiveTypeInfo;

struct PrimitiveTypeInfoData
{
    TypeInfoData basic;
    const void* defaultValue;
};

class PrimitiveTypeInfo : public TypeInfo
{
public:
    FB_REFL_API         PrimitiveTypeInfo(const void* infoData = nullptr);
    FB_REFL_API virtual ~PrimitiveTypeInfo();

    ImplementInfoDataGetter(PrimitiveTypeInfoData)

    const void* getDefaultValue() const
        { return getPrimitiveTypeInfoData()->defaultValue; }

    const void* getDefaultInstance() const override
        { return getDefaultValue(); }
    
    void* createInstance() const { return nullptr; }

    SkeletonNode* toSkel(const void* data, const SkelConversionContext* context) const override;
    void toString(const void* data, const StringConversionParams* params, StringEditor& result) const override;
};

#define STD_PRIMITIVE_DECL(nativetyp,typ) \
class typ ## TypeInfo : public ::rw::PrimitiveTypeInfo { \
public: \
typ ## TypeInfo(void* infoData) : ::rw::PrimitiveTypeInfo((PrimitiveTypeInfoData*)infoData) {} \
REFL_TYPEINFO_MEMBERTYPE(kTypeCode_ ## typ, kTypeCategory_Member, 0) \
REFL_TYPEINFO_DECL(REFL_TYPEINFO_PRIV_FUNCS REFL_TYPEINFO_PRIV_ARRAY_FUNCS REFL_TYPEINFO_PUBL_FUNCS, \
    REFL_TYPEINFO_TYPEDEFS(Core, typ ## TypeInfo, PrimitiveTypeInfo, PrimitiveTypeInfoData)) \
    typedef nativetyp Native_t; \
}; \
template<> class _TypeHelper<nativetyp> : public typ ## TypeInfo {};

//STD_PRIMITIVE_DECL(void, Void)
STD_PRIMITIVE_DECL(bool, Bool)
STD_PRIMITIVE_DECL(s8, Int8)
STD_PRIMITIVE_DECL(u8, Uint8)
STD_PRIMITIVE_DECL(s16, Int16)
STD_PRIMITIVE_DECL(u16, Uint16)
STD_PRIMITIVE_DECL(s32, Int32)
STD_PRIMITIVE_DECL(u32, Uint32)
STD_PRIMITIVE_DECL(s64, Int64)
STD_PRIMITIVE_DECL(u64, Uint64)
STD_PRIMITIVE_DECL(f32, Float32)
STD_PRIMITIVE_DECL(f64, Float64)
STD_PRIMITIVE_DECL(Guid, Guid)
STD_PRIMITIVE_DECL(SHA1, Sha1)
STD_PRIMITIVE_DECL(String, CString)
}