#ifndef _ENGINE_SYMBOL_MACROS_H

#include "ReflectionMacros.h"

#define DECLARE_SYMBOL(modul, typ, basetyp) REFL_CLASSINFO_DECL(modul, typ, basetyp, SymbolClassInfo, ClassInfoData)
#define DECLARE_ABSTRACT_SYMBOL(modul, typ, basetyp) REFL_CLASSINFO_ABSTRACT_DECL(modul, typ, basetyp, SymbolClassInfo, ClassInfoData)

#define _FUNCS This_t::createInstance_, This_t::copyInstance_, This_t::destroyInstance_
#define _NULLFUNCS nullptr, nullptr, nullptr
#define _ARRAYFUNCS This_t::setElementCount_, This_t::releaseElements_
#define _NULLARRAYFUNCS nullptr, nullptr

#define REFL_IMPLEMENT_SYMBOL(typ, funcs, arrayfuncs) \
REFL_TYPEINFO_EXPORT(typ, REFL_CLASSINFO_BLOCK( \
    REFL_TYPEINFO_BLOCK( \
        REFL_MEMBERINFO_BLOCK( \
            #typ, \
            REFL_HASH_CONSTEXPR(#typ), \
            sizeof(void*), \
            alignof(void*), \
            REFL_MEMBERINFO_FLAGS(kTypeCode_Void, kTypeCategory_Class, 0), \
            This_t::typedata::c_attributes, \
            sizeof_array(This_t::typedata::c_attributes) - 1 \
        ), \
        ::rw::genTypeGuid(Module_t::c_moduleInfo.getName(), #typ), \
        &Module_t::c_moduleInfo, \
        This_t::typedata::c_fieldInfos, \
        sizeof_array(This_t::typedata::c_fieldInfos) - 1, \
        sizeof(typ), \
        alignof(typ), \
        funcs \
    ), \
    &Base_t::typedata::c_typeInfo, \
    nullptr, nullptr, nullptr, nullptr \
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
    arrayfuncs \
) \
)

#define EXPORT_SYMBOL(typ) REFL_IMPLEMENT_SYMBOL(typ, _FUNCS, _ARRAYFUNCS)
#define EXPORT_ABSTRACT_SYMBOL(typ) REFL_IMPLEMENT_SYMBOL(typ, _NULLFUNCS, _NULLARRAYFUNCS)

#endif