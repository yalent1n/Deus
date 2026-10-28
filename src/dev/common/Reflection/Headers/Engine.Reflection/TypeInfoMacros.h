#pragma once

#define REFL_MEMBERINFO_FLAGS(code, category, flags) \
    { code, category, flags }

#define REFL_MEMBERINFO_BLOCK(name, namehash, size, alignment, flags, attributes, attributecount) \
    { name, attributes, namehash, size, alignment, flags, attributecount }

#define REFL_TYPEINFO_BLOCK(memberinfo, guid, moduleptr, fieldinfos, fieldcount, size, alignment, createfun, copyfun, destroyfun) \
    { memberinfo, guid, moduleptr, fieldinfos, fieldcount, size, alignment, createfun, copyfun, destroyfun }

#define REFL_TYPEINFO_MEMBERTYPE(code, category, flags) \
enum : rw::u8 \
{ \
    kMemberTypeCode = code, \
    kMemberTypeCategory = category, \
    kMemberTypeFlags = flags \
};

#define REFL_TYPEINFO_TYPEDEFS(modul, typ, infotyp, infodatatyp) \
public: \
typedef modul ## _Module Module_t; \
typedef typ This_t; \
typedef infotyp TypeInfo_t; \
typedef infodatatyp TypeInfoData_t;

#define REFL_TYPEINFO_PRIV_FUNCS \
private: \
static void createInstance_(void* placement); \
static void copyInstance_(void* placement, void* otherPlacement); \
static void destroyInstance_(void* placement);

#define REFL_TYPEINFO_PRIV_ARRAY_FUNCS \
private: \
static void setElementCount_(void* placement, u32 count); \
static void releaseElements_(void* placement);

#define REFL_TYPEINFO_PUBL_FUNCS \
public: \
static LIB_LINKAGE This_t* createInstance(); \
static LIB_LINKAGE void copyInstance(This_t* placement, This_t* otherPlacement); \
static LIB_LINKAGE void destroyInstance(This_t* placement);

#define REFL_TYPEINFO_FUNCS_UNDEFINED(typ) \
void typ::createInstance_(void*) { EASSERT(false, #typ"::createInstance_ not implemented"); } \
void typ::copyInstance_(void*, void*) { EASSERT(false, #typ"::copyInstance_ not implemented"); } \
void typ::destroyInstance_(void*) { EASSERT(false, #typ"::destroyInstance_ not implemented"); } \
typ::This_t* typ::createInstance() { EASSERT(false, #typ"::createInstance not implemented"); } \
void typ::copyInstance(typ::This_t*, typ::This_t*) { EASSERT(false, #typ"::copyInstance not implemented"); } \
void typ::destroyInstance(typ::This_t*) { EASSERT(false, #typ"::destroyInstance not implemented"); }

#define REFL_TYPEINFO_ARRAY_FUNCS_UNDEFINED(typ) \
void typ::setElementCount_(void*, u32) { EASSERT(false, #typ"::setElementCount_ not implemented"); } \
void typ::releaseElements_(void*) { EASSERT(false, #typ"::releaseElements_ not implemented"); }

#define REFL_TYPEINFO_FUNCS_ABSTRACT(typ) \
void typ::createInstance_(void*) { EASSERT(false, #typ"::createInstance_ abstract class"); } \
void typ::copyInstance_(void*, void*) { EASSERT(false, #typ"::copyInstance_ abstract class"); } \
void typ::destroyInstance_(void*) { EASSERT(false, #typ"::destroyInstance_ abstract class"); } \
typ::This_t* typ::createInstance() { EASSERT(false, #typ"::createInstance abstract class"); } \
void typ::copyInstance(typ::This_t*, typ::This_t*) { EASSERT(false, #typ"::copyInstance abstract class"); } \
void typ::destroyInstance(typ::This_t*) { EASSERT(false, #typ"::destroyInstance abstract class"); }

#define REFL_TYPEINFO_ARRAY_FUNCS_ABSTRACT(typ) \
void typ::setElementCount_(void*, u32) { EASSERT(false, #typ"::setElementCount_ abstract class"); } \
void typ::releaseElements_(void*) { EASSERT(false, #typ"::releaseElements_ abstract class"); }

#define REFL_TYPEINFO_FUNCS_NOCOPY(typ) \
void typ::createInstance_(void* placement) { new (placement) typ; } \
void typ::copyInstance_(void*, void*) { EASSERT(false, #typ"::copyInstance_ not implemented"); } \
void typ::destroyInstance_(void* placement) { ((typ*)placement)->~typ(); } \
typ::This_t* typ::createInstance() { return (typ::This_t*) typedata::c_typeInfo.createInstance(); } \
void typ::copyInstance(typ::This_t*, typ::This_t*) { EASSERT(false, #typ"::copyInstance not implemented"); } \
void typ::destroyInstance(typ::This_t*) {  }

#define REFL_TYPEINFO_FUNCS(typ) \
void typ::createInstance_(void* placement) { new (placement) typ; } \
void typ::copyInstance_(void* placement, void* otherPlacement) { new (placement) typ(*(typ*)otherPlacement); } \
void typ::destroyInstance_(void* placement) { ((typ*)placement)->~typ(); } \
typ::This_t* typ::createInstance() { return (typ::This_t*) typedata::c_typeInfo.createInstance(); } \
void typ::copyInstance(typ::This_t*, typ::This_t*) { ELOGF("copy instance"); } \
void typ::destroyInstance(typ::This_t*) {  }

#define REFL_TYPEINFO_ARRAY_FUNCS(typ) \
void typ::setElementCount_(void* placement, u32 count) { ((::rw::Array<typ>*)placement)->resize(count); } \
void typ::releaseElements_(void* placement) { ((::rw::Array<typ>*)placement)->clear(); }

#define REFL_TYPEINFO_DECL(typfuncs, typdefs) \
typdefs \
typfuncs \
public: \
struct typedata { \
static LIB_LINKAGE const TypeInfo_t c_typeInfo; \
static LIB_LINKAGE const TypeInfoData_t c_typeInfoData; \
static LIB_LINKAGE const ArrayTypeInfo c_arrayTypeInfo; \
static LIB_LINKAGE const ArrayTypeInfoData c_arrayInfoData; \
static const FieldInfoData c_fieldInfos[]; \
static const AttributeSpec c_attributes[]; \
};

#define REFL_TYPEINFO_EXPORT(typ, infodata, arrayinfodata) \
const typ::TypeInfoData_t typ::typedata::c_typeInfoData = infodata; \
const typ::TypeInfo_t typ::typedata::c_typeInfo = &typ::typedata::c_typeInfoData; \
const rw::ArrayTypeInfoData typ::typedata::c_arrayInfoData = arrayinfodata; \
const rw::ArrayTypeInfo typ::typedata::c_arrayTypeInfo = &typ::typedata::c_arrayInfoData;

#define BEGIN_TYPE_ATTRIBUTES_EXPORT(typ) \
    const rw::AttributeSpec typ::typedata::c_attributes[] = {

#define BEGIN_MEMBER_ATTRIBUTES_EXPORT(typ, member) \
    const rw::AttributeSpec typ::c_ ## member ## Attributes[] = {

#define END_ATTRIBUTES_EXPORT() {} };

#define EXPORT_ATTRIBUTE(nam, val) { nam, val },