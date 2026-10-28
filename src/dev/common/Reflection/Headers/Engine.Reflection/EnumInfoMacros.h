#pragma once
#include "TypeInfoMacros.h"
#include "FieldInfoMacros.h"

// BEGIN_ENUM_MEMBERS(Platform)
//      ENUM_MEMBER(...)
// END_ENUM_MEMBERS()
//
// FB_DECLARE_ENUM(Core, Platform)
// FB_IMPLEMENT_ENUM(Platform)

#define REFL_ENUM_FUNCS(typ) \
void typ ## TypeInfo::createInstance_(void* placement) { new (placement) typ; } \
void typ ## TypeInfo::copyInstance_(void* placement, void* otherPlacement) { *(typ*)placement = *(typ*)otherPlacement; } \
void typ ## TypeInfo::destroyInstance_(void* placement) {  } \
typ ## TypeInfo::This_t* typ ## TypeInfo::createInstance() { return (typ ## TypeInfo::This_t*) new typ; } \
void typ ## TypeInfo::copyInstance(typ ## TypeInfo::This_t* lhs, typ ## TypeInfo::This_t* rhs) { *(typ*)lhs = *(typ*)rhs; } \
void typ ## TypeInfo::destroyInstance(typ ## TypeInfo::This_t*) {  }

#define REFL_ENUM_ARRAY_FUNCS(typ) \
void typ ## TypeInfo::setElementCount_(void* placement, u32 count) { ((::rw::Array<typ>*)placement)->resize(count); } \
void typ ## TypeInfo::releaseElements_(void* placement) { ((::rw::Array<typ>*)placement)->clear(); }

#define REFL_ENUM_TYPEINFO_BLOCK(basic, def) { basic, def }

#define REFL_DECLARE_ENUM(modul, typ, defaultValue) \
class typ ## TypeInfo : public ::rw::EnumTypeInfo \
{ \
public: \
    typ ## TypeInfo(const void* infoData) : ::rw::EnumTypeInfo((const EnumTypeInfoData*)infoData) {} \
REFL_TYPEINFO_MEMBERTYPE(kTypeCode_Int32, kTypeCategory_Enum, 0) \
REFL_TYPEINFO_DECL(REFL_TYPEINFO_PRIV_FUNCS REFL_TYPEINFO_PRIV_ARRAY_FUNCS REFL_TYPEINFO_PUBL_FUNCS, \
    REFL_TYPEINFO_TYPEDEFS(modul, typ ## TypeInfo, EnumTypeInfo, EnumTypeInfoData)) \
    typedef int Native_t; \
    static inline constexpr int c_defaultValue = defaultValue; \
}; \
template<> class _TypeHelper<typ> : public typ ## TypeInfo {};

#define REFL_BEGIN_ENUM(en) BEGIN_FIELDS_EXPORT(en ## TypeInfo)
#define REFL_EXPORT_ENUM_MEMBER(ev) \
REFL_FIELDINFO_BLOCK \
( \
    REFL_MEMBERINFO_BLOCK \
    ( \
        #ev, \
        REFL_HASH_CONSTEXPR(#ev), \
        sizeof(ev), \
        alignof(ev), \
        REFL_MEMBERINFO_FLAGS(kTypeCode_Int32, kTypeCategory_Member, 0), \
        c_enum_ ## ev ## Attributes, \
        sizeof_array(c_enum_ ## ev ## Attributes) - 1 \
    ), \
    ::rw::Type<int>(), \
    &This_t::typedata::c_typeInfo, \
    (u32)ev \
),
#define REFL_END_ENUM() END_FIELDS_EXPORT()

#define REFL_IMPLEMENT_ENUM(typ) \
REFL_ENUM_FUNCS(typ) \
REFL_ENUM_ARRAY_FUNCS(typ) \
REFL_TYPEINFO_EXPORT(typ ## TypeInfo, \
    REFL_ENUM_TYPEINFO_BLOCK( \
        REFL_TYPEINFO_BLOCK \
        ( \
            REFL_MEMBERINFO_BLOCK \
            ( \
                #typ, REFL_HASH_CONSTEXPR(#typ), sizeof(typ), alignof(typ), REFL_MEMBERINFO_FLAGS(kTypeCode_Int32, kTypeCategory_Enum, 0), \
                This_t::typedata::c_attributes, \
                sizeof_array(This_t::typedata::c_attributes) - 1 \
            ), \
            genTypeGuid(Module_t::c_moduleInfo.getName(), #typ), \
            &Module_t::c_moduleInfo, \
            This_t::typedata::c_fieldInfos, \
            sizeof_array(This_t::typedata::c_fieldInfos) - 1, \
            sizeof(typ), \
            alignof(typ), \
            This_t::createInstance_, This_t::copyInstance_, This_t::destroyInstance_ \
        ), \
        c_defaultValue \
    ), REFL_ARRAY_TYPEINFO_BLOCK( \
        REFL_TYPEINFO_BLOCK( \
            REFL_MEMBERINFO_BLOCK( \
                #typ "[]", \
                REFL_HASH_CONSTEXPR(#typ "[]"), \
                sizeof(void*), \
                alignof(void*), \
                REFL_MEMBERINFO_FLAGS(kTypeCode_Void, kTypeCategory_Array, 0), \
                nullptr, \
                0 \
            ), \
            ::rw::genArrayTypeGuid(Module_t::c_moduleInfo.getName(), #typ), \
            &Module_t::c_moduleInfo, \
            nullptr, \
            0, \
            sizeof(void*), \
            alignof(void*), \
            nullptr, nullptr, nullptr \
        ), \
        &This_t::typedata::c_typeInfo, \
        This_t::setElementCount_, \
        This_t::releaseElements_ \
    ) \
)

#define DECLARE_ENUM(modul, nam, def) REFL_DECLARE_ENUM(modul, nam, def)
#define EXPORT_ENUM(nam) REFL_IMPLEMENT_ENUM(nam)

#define BEGIN_ENUM_EXPORT(nam) REFL_BEGIN_ENUM(nam)
#define END_ENUM_EXPORT() REFL_END_ENUM()
#define EXPORT_ENUM_MEMBER(en) REFL_EXPORT_ENUM_MEMBER(en)

#define BEGIN_ENUM_ATTRIBUTES_EXPORT(typ) \
    const rw::AttributeSpec typ ## TypeInfo::typedata::c_attributes[] = {

#define BEGIN_ENUM_MEMBER_ATTRIBUTES_EXPORT(member) \
    const rw::AttributeSpec c_enum_ ## member ## Attributes[] = {