#pragma once
#include "FieldInfoMacros.h"
#include "TypeInfoMacros.h"

#define REFL_CLASSINFO_BLOCK(basic, baseclass, createreffun, setreffun, destroyreffun, resolvereffun) \
    { basic, baseclass, createreffun, setreffun, destroyreffun, resolvereffun }

#define REFL_CLASSINFO_TYPEDEFS(modul, typ, basetyp, infotyp, infodatatyp) \
REFL_TYPEINFO_TYPEDEFS(modul, typ, infotyp, infodatatyp) \
typedef basetyp Base_t;

#define REFL_CLASSINFO_DECL(modul, typ, basetyp, infotyp, infodatatyp) \
REFL_TYPEINFO_DECL(REFL_TYPEINFO_PRIV_FUNCS REFL_TYPEINFO_PRIV_ARRAY_FUNCS REFL_TYPEINFO_PUBL_FUNCS, \
    REFL_CLASSINFO_TYPEDEFS(modul, typ, basetyp, infotyp, infodatatyp) \
)

#define REFL_CLASSINFO_ABSTRACT_DECL(modul, typ, basetyp, infotyp, infodatatyp) \
REFL_TYPEINFO_DECL(, REFL_CLASSINFO_TYPEDEFS(modul, typ, basetyp, infotyp, infodatatyp))
