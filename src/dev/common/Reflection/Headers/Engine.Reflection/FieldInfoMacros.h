#pragma once
#include "TypeInfoMacros.h"

#define REFL_FIELDINFO_BLOCK(memberinfo, fieldtyp, declaringtyp, fieldoffset) \
    { memberinfo, fieldtyp, declaringtyp, fieldoffset }

#define REFL_FIELDINFO_EX(fieldtyp, propnam, nam, inop, intyp, outop, outtyp, outparams, outfunc) \
private: \
static const AttributeSpec c_ ## propnam ## Attributes[]; \
fieldtyp m_ ## nam; \
public: \
intyp inop ## propnam() const { return m_##nam; } \
outtyp outop ## propnam(outparams) { outfunc }

#define REFL_MEMBER_EX(typ, nam, propnam) REFL_FIELDINFO_EX(typ, nam, propnam, get, typ, set, void, const typ& _Value, m_##nam = _Value;)
#define REFL_ARRAY_MEMBER_EX(typ, nam, propnam) REFL_FIELDINFO_EX(Array<typ>, nam, propnam, read, ::rw::ArrayReader<typ>, edit, ::rw::ArrayEditor<typ>, , return m_##nam;)
#define DECLARE_MEMBER(typ, nam) REFL_MEMBER_EX(typ, nam, nam)
#define DECLARE_ARRAY_MEMBER(typ, nam) REFL_ARRAY_MEMBER_EX(typ, nam, nam)

#define BEGIN_FIELDS_EXPORT(typ) \
    const rw::FieldInfoData typ::typedata::c_fieldInfos[] = {

#define END_FIELDS_EXPORT() {} };

#define EXPORT_MEMBER(nam) \
REFL_FIELDINFO_BLOCK \
( \
    REFL_MEMBERINFO_BLOCK \
    ( \
        #nam, \
        REFL_HASH_CONSTEXPR(#nam), \
        sizeof(This_t::m_##nam), \
        alignof(This_t::m_##nam), \
        REFL_MEMBERINFO_FLAGS(kTypeCode_Void, kTypeCategory_Member, 0), \
        This_t::c_ ## nam ## Attributes, \
        sizeof_array(This_t::c_ ## nam ## Attributes) - 1 \
    ), \
    ::rw::Type(((This_t*)nullptr)->m_##nam), \
    &This_t::typedata::c_typeInfo, \
    (u16)offsetof(This_t, m_##nam) \
),

#define EXPORT_ARRAY_MEMBER(nam) \
REFL_FIELDINFO_BLOCK \
( \
    REFL_MEMBERINFO_BLOCK \
    ( \
        #nam, \
        REFL_HASH_CONSTEXPR(#nam), \
        sizeof(This_t::m_##nam), \
        alignof(This_t::m_##nam), \
        REFL_MEMBERINFO_FLAGS(kTypeCode_Void, kTypeCategory_Member, 0), \
        This_t::c_ ## nam ## Attributes, \
        sizeof_array(This_t::c_ ## nam ## Attributes) - 1 \
    ), \
    ::rw::Type(((This_t*)nullptr)->m_##nam), \
    &This_t::typedata::c_typeInfo, \
    (u16)offsetof(This_t, m_##nam) \
),

#define EXPORT_STATIC_MEMBER(nam, flags) \
REFL_FIELDINFO_BLOCK \
( \
    REFL_MEMBERINFO_BLOCK \
    ( \
        #nam, \
        REFL_HASH_CONSTEXPR(#nam), \
        sizeof(This_t::nam), \
        alignof(This_t::nam), \
        REFL_MEMBERINFO_FLAGS(rw::kTypeCode_Void, rw::kTypeCategory_Member, flags | rw::MemberInfoFlags::STATIC), \
        This_t::c_ ## nam ## Attributes, \
        sizeof_array(This_t::c_ ## nam ## Attributes) - 1 \
    ), \
    ::rw::Type(((This_t*)nullptr)->m_##nam), \
    &This_t::typedata::c_typeInfo, \
    (u16)offsetof(This_t, m_##nam) \
)