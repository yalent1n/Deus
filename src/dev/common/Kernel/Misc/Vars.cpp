#include <Engine.Base/Misc/Vars.h>

namespace rw
{
static VarsVar* s_firstVar = nullptr;

VarsVar::VarsVar(const char* key_, void* rawValue_, u8 memberSize_, VarT type_, const char* tag_)
    : key(key_)
    , rawValue(rawValue_)
    , memberSize(memberSize_)
    , type(type_)
    , tag(tag_)
{
    nextVar = s_firstVar;
    s_firstVar = this;
}

VarsVar* Vars::getVar(const char* key)
{
    VarsVar* var = s_firstVar;
    if(var)
    {
        while(var)
        {
            if(equalsIgnoreCase(var->key, key))
                return var;

            var = var->nextVar;
        }
    }

    return nullptr;
}

void* Vars::getRawValue(const char* key)
{
    VarsVar* var = getVar(key);
    return var ? var->rawValue : nullptr;
}

s64 Vars::getInteger(const char* key, s64 defaultValue)
{
    VarsVar* var = getVar(key);
    if(!var)
        return defaultValue;

    if(!((u8)var->type & (u8)VarT::Integer))
        return defaultValue;

    if(var)
    {
        switch(var->memberSize)
        {
            case 1: return (((u8)var->type & (u8)VarT::SignedFlag) ? s64(*(s8*)var->rawValue) : s64(*(u8*)var->rawValue));
            case 2: return (((u8)var->type & (u8)VarT::SignedFlag) ? s64(*(s16*)var->rawValue) : s64(*(u16*)var->rawValue));
            case 4: return (((u8)var->type & (u8)VarT::SignedFlag) ? s64(*(s32*)var->rawValue) : s64(*(u32*)var->rawValue));
            case 8: return (((u8)var->type & (u8)VarT::SignedFlag) ? *(s64*)var->rawValue : s64(*(u64*)var->rawValue));
            default: return defaultValue;
        }
    }

    return defaultValue;
}

f64 Vars::getFloat(const char* key, f64 defaultValue)
{
    VarsVar* var = getVar(key);
    if(!var)
        return defaultValue;

    if(!((u8)var->type & (u8)VarT::Float))
        return defaultValue;

    if(var)
    {
        switch(var->memberSize)
        {            
            case 4: return *(f32*)var->rawValue;
            case 8: return *(f64*)var->rawValue;
            default: return defaultValue;
        }
    }

    return defaultValue;
}

const char* Vars::getString(const char* key, const char* defaultValue)
{
    VarsVar* var = getVar(key);
    if(!var)
        return defaultValue;

    return var ? *(const char**)var->rawValue : defaultValue;
}

bool Vars::getBoolean(const char* key, bool defaultValue)
{
    VarsVar* var = getVar(key);
    if(!var)
        return defaultValue;

    if(!((u8)var->type & (u8)VarT::Boolean))
        return 0;

    return var ? *(bool*)var->rawValue : defaultValue;    
}
}