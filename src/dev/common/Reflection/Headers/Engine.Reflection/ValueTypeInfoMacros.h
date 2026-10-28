#pragma once

#define REFL_VALUETYPEINFO_BLOCK(basic) { basic, nullptr }

#define REFL_VALUETYPEINFO_TYPEDEFS(modul, typ) \
REFL_TYPEINFO_TYPEDEFS(modul, typ, ::rw::ValueTypeInfo, ::rw::ValueTypeInfoData)

#define REFL_VALUETYPEINFO_DECL(modul, typ) \
REFL_TYPEINFO_DECL( \
    REFL_TYPEINFO_PRIV_FUNCS REFL_TYPEINFO_PRIV_ARRAY_FUNCS REFL_TYPEINFO_PUBL_FUNCS, \
    REFL_VALUETYPEINFO_TYPEDEFS(modul, typ) \
)

#define REFL_VALUETYPEINFO_EXPORT(typ) REFL_TYPEINFO_EXPORT(typ, REFL_VALUETYPEINFO_BLOCK( \
    REFL_TYPEINFO_BLOCK( \
        REFL_MEMBERINFO_BLOCK( \
            #typ, \
            REFL_HASH_CONSTEXPR(#typ), \
            sizeof(typ), \
            alignof(typ), \
            REFL_MEMBERINFO_FLAGS(kTypeCode_Void, kTypeCategory_ValueType, 0), \
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
    ) \
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

#define DECLARE_STRUCT(modul, typ) REFL_VALUETYPEINFO_DECL(modul, typ)
#define EXPORT_STRUCT(typ) REFL_VALUETYPEINFO_EXPORT(typ)