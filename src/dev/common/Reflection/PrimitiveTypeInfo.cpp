#include <Engine.Reflection/PrimitiveTypeInfo.h>
#include <Engine.Reflection/Array.h>

#define STD_FUNCS_DEFAULT(nativetyp, typ) \
void typ::createInstance_(void* placement) { new (placement) nativetyp; } \
void typ::copyInstance_(void* placement, void* otherPlacement) { *(nativetyp*)placement = *(nativetyp*)otherPlacement; } \
void typ::destroyInstance_(void* placement) {  } \
typ::This_t* typ::createInstance() { return reinterpret_cast<typ::This_t*>(new nativetyp); } \
void typ::copyInstance(typ::This_t* lhs, typ::This_t* rhs) { *(nativetyp*)lhs = *(nativetyp*)rhs; } \
void typ::destroyInstance(typ::This_t*) {  }

#define STD_ARRAY_FUNCS_DEFAULT(nativetyp, typ) \
void typ::setElementCount_(void* placement, u32 count) { ((::rw::Array<nativetyp>*)placement)->resize(count); } \
void typ::releaseElements_(void* placement) { ((::rw::Array<nativetyp>*)placement)->clear(); }

namespace rw
{
template<typename T>
struct PrimitiveTypeHelper
{
    static SkeletonNode* toSkel(const void* data, const TypeInfo::SkelConversionContext* context)
    {
        SkeletonNode* node = nullptr;

        if(!data)
            return nullptr;

        node = context->skel->createElement(context->asArray ? "member" : "field");
        node = context->root->appendChild(node);

        StringBuilder<32> text;
        PrimitiveTypeHelper<T>::toString(data, text);

        if(!text.isEmpty())
            node->setText(text.detach());

        return node;
    }

    static void toString(const void* data, StringEditor& result)
    {
        if(!data)
            return;
            
        ::rw::tostring(*(T*)data, result);
    }
};

template<>
struct PrimitiveTypeHelper<bool>
{
    static SkeletonNode* toSkel(const void* data, const TypeInfo::SkelConversionContext* context)
    {
        SkeletonNode* node = nullptr;

        if(!data)
            return nullptr;

        node = context->skel->createElement("field");
        node = context->root->appendChild(node);

        StringBuilder<32> text;
        PrimitiveTypeHelper<bool>::toString(data, text);

        if(!text.isEmpty())
            node->setText(text.detach());

        return node;
    }

    static void toString(const void* data, StringEditor& result)
    {
        result.appendChars(*(const bool*)data ? "true" : "false");
    }
};

template<>
struct PrimitiveTypeHelper<String>
{
    static SkeletonNode* toSkel(const void* data, const TypeInfo::SkelConversionContext* context)
    {
        SkeletonNode* node = nullptr;

        if(!data)
            return nullptr;

        node = context->skel->createElement("field");
        node = context->root->appendChild(node);

        StringBuilder<32> text;
        PrimitiveTypeHelper<String>::toString(data, text);

        if(!text.isEmpty())
            node->setText(text.detach());

        return node;
    }

    static void toString(const void* data, StringEditor& result)
    {
        result.appendChars(((String*)data)->c_str());
    }
};

template<>
struct PrimitiveTypeHelper<Guid>
{
    static SkeletonNode* toSkel(const void* data, const TypeInfo::SkelConversionContext* context)
    {
        SkeletonNode* node = nullptr;

        if(!data)
            return nullptr;

        node = context->skel->createElement("field");
        node = context->root->appendChild(node);

        StringBuilder<40> text;
        PrimitiveTypeHelper<Guid>::toString(data, text);

        if(!text.isEmpty())
            node->setText(text.detach());

        return node;
    }

    static void toString(const void* data, StringEditor& result)
    {
        result.appendChars(GuidStr(*(const Guid*)data).str);
    }
};

template<>
struct PrimitiveTypeHelper<SHA1>
{
    static SkeletonNode* toSkel(const void* data, const TypeInfo::SkelConversionContext* context)
    {
        SkeletonNode* node = nullptr;

        if(!data)
            return nullptr;

        node = context->skel->createElement("field");
        node = context->root->appendChild(node);

        StringBuilder<41> text;
        PrimitiveTypeHelper<SHA1>::toString(data, text);

        if(!text.isEmpty())
            node->setText(text.detach());

        return node;
    }

    static void toString(const void* data, StringEditor& result)
    {
        tohexstr(data, sizeof(SHA1), result);
    }
};

typedef SkeletonNode* (*ToSkeletonFunc)(const void* data, const TypeInfo::SkelConversionContext* context);
typedef void (*ToStringFunc)(const void* data, StringEditor& result);

struct PrimitiveTypeHandler
{
    ToSkeletonFunc      toSkel;
    ToStringFunc        toString;
};

#define STD_PRIMITIVE_HANDLER_(nativetyp, typ) { PrimitiveTypeHelper<nativetyp>::toSkel, PrimitiveTypeHelper<nativetyp>::toString }

static PrimitiveTypeHandler g_typeConversionTable[] = {
    { nullptr, nullptr }, /*void*/
    STD_PRIMITIVE_HANDLER_(bool, kTypeCode_Bool),
    STD_PRIMITIVE_HANDLER_(s8, kTypeCode_Int8),
    STD_PRIMITIVE_HANDLER_(u8, kTypeCode_Uint8),
    STD_PRIMITIVE_HANDLER_(s16, kTypeCode_Int16),
    STD_PRIMITIVE_HANDLER_(u16, kTypeCode_Uint16),
    STD_PRIMITIVE_HANDLER_(s32, kTypeCode_Int32),
    STD_PRIMITIVE_HANDLER_(u32, kTypeCode_Uint32),
    STD_PRIMITIVE_HANDLER_(s64, kTypeCode_Int64),
    STD_PRIMITIVE_HANDLER_(u64, kTypeCode_Uint64),
    STD_PRIMITIVE_HANDLER_(f32, kTypeCode_Float32),
    STD_PRIMITIVE_HANDLER_(f64, kTypeCode_Float64),
    STD_PRIMITIVE_HANDLER_(Guid, kTypeCode_Guid),
    STD_PRIMITIVE_HANDLER_(SHA1, kTypeCode_Sha1),
    STD_PRIMITIVE_HANDLER_(String, kTypeCode_CString)
};

#undef STD_PRIMITIVE_TYPEINFO_

PrimitiveTypeInfo::PrimitiveTypeInfo(const void* infoData)
    : TypeInfo(&((PrimitiveTypeInfoData*)infoData)->basic)
{}

PrimitiveTypeInfo::~PrimitiveTypeInfo()
{}

SkeletonNode* PrimitiveTypeInfo::toSkel(const void* data, const SkelConversionContext* context) const
{
    return g_typeConversionTable[getMemberInfoData()->flags.typeCode].toSkel(data, context);
}

void PrimitiveTypeInfo::toString(const void* data, const StringConversionParams* params, StringEditor& result) const
{
    return g_typeConversionTable[getMemberInfoData()->flags.typeCode].toString(data, result);
}

#define STD_PRIMITIVE_TYPEINFO_BLOCK(typedata, defaultValue) \
{ typedata, defaultValue }

#define STD_PRIMITIVE_IMPL(nativetyp, typ) \
STD_FUNCS_DEFAULT(nativetyp, typ ## TypeInfo) \
STD_ARRAY_FUNCS_DEFAULT(nativetyp, typ ## TypeInfo) \
REFL_TYPEINFO_EXPORT(typ ## TypeInfo, \
    STD_PRIMITIVE_TYPEINFO_BLOCK \
    ( \
        REFL_TYPEINFO_BLOCK \
        ( \
            REFL_MEMBERINFO_BLOCK( \
                #typ, REFL_HASH_CONSTEXPR(#typ), sizeof(nativetyp), alignof(nativetyp), REFL_MEMBERINFO_FLAGS(kTypeCode_ ## typ, kTypeCategory_Member, 0), \
                nullptr, 0 \
            ), \
            genTypeGuid(Module_t::c_moduleInfo.getName(), #typ), \
            &Module_t::c_moduleInfo, \
            nullptr, \
            0, \
            sizeof(nativetyp), \
            alignof(nativetyp), \
            This_t::createInstance_, This_t::copyInstance_, This_t::destroyInstance_ \
        ), \
        nullptr \
    ), \
    REFL_ARRAY_TYPEINFO_BLOCK( \
    REFL_TYPEINFO_BLOCK( \
        REFL_MEMBERINFO_BLOCK( \
            #typ "[]", \
            REFL_HASH_CONSTEXPR(#typ "[]"), \
            sizeof(void*), \
            alignof(void*), \
            REFL_MEMBERINFO_FLAGS(kTypeCode_Void, kTypeCategory_Array, 0), \
            nullptr, 0 \
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

//STD_PRIMITIVE_IMPL(void, Void)
STD_PRIMITIVE_IMPL(bool, Bool)
STD_PRIMITIVE_IMPL(s8, Int8)
STD_PRIMITIVE_IMPL(u8, Uint8)
STD_PRIMITIVE_IMPL(s16, Int16)
STD_PRIMITIVE_IMPL(u16, Uint16)
STD_PRIMITIVE_IMPL(s32, Int32)
STD_PRIMITIVE_IMPL(u32, Uint32)
STD_PRIMITIVE_IMPL(s64, Int64)
STD_PRIMITIVE_IMPL(u64, Uint64)
STD_PRIMITIVE_IMPL(f32, Float32)
STD_PRIMITIVE_IMPL(f64, Float64)
STD_PRIMITIVE_IMPL(Guid, Guid)
STD_PRIMITIVE_IMPL(SHA1, Sha1)
STD_PRIMITIVE_IMPL(String, CString)
}