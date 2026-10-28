#pragma once
#include "../Types.h"
#include "../String/Util.h"

#define XVARIABLE(T, var, k) \
static T g_##var; \
inline rw::VarsVarT<T> __var_##var(k, g_##var, rw::toVarT<T>())

#if defined(ENGINE_VARS_MODULE)
    #define FB_VARS_API LIB_LINKAGE
#else
    #define FB_VARS_API
#endif

namespace rw
{
enum class VarT : u8
{
    Unknown = 0x00,
    Integer = 0x01,
    Float = 0x02,
    String = 0x04,
    Boolean = 0x08,

    SignedFlag = 0x80
};

template<typename T> inline VarT toVarT() { return VarT::Unknown; }

// Integers
template<> inline VarT toVarT<s8>() { return VarT((u8)VarT::Integer | (u8)VarT::SignedFlag); }
template<> inline VarT toVarT<u8>() { return VarT::Integer; }
template<> inline VarT toVarT<s16>() { return VarT((u8)VarT::Integer | (u8)VarT::SignedFlag); }
template<> inline VarT toVarT<u16>() { return VarT::Integer; }
template<> inline VarT toVarT<s32>() { return VarT((u8)VarT::Integer | (u8)VarT::SignedFlag); }
template<> inline VarT toVarT<u32>() { return VarT::Integer; }
template<> inline VarT toVarT<s64>() { return VarT((u8)VarT::Integer | (u8)VarT::SignedFlag); }
template<> inline VarT toVarT<u64>() { return VarT::Integer; }

// Floating-point numbers
template<> inline VarT toVarT<f32>() { return VarT::Float; }
template<> inline VarT toVarT<f64>() { return VarT::Float; }

template<> inline VarT toVarT<const char*>() { return VarT::String; }
template<> inline VarT toVarT<bool>() { return VarT::Boolean; }

struct VarsVar
{
    const char* key;
    void* rawValue;
    u8 memberSize;
    VarT type;
    const char* tag;
    VarsVar* nextVar;

    LIB_LINKAGE VarsVar(const char* key_, void* rawValue, u8 memberSize, VarT type, const char* tag);
};

template<typename T>
class VarsVarT : VarsVar
{
public:
    VarsVarT(const char* key, T& rawValue, VarT type, const char* tag = nullptr);
};

template<typename T>
VarsVarT<T>::VarsVarT(const char* key, T& rawValue, VarT type, const char* tag)
    : VarsVar(key, &rawValue, sizeof(T), type, tag)
{}

class Vars
{
public:
    LIB_LINKAGE static VarsVar*         getVar(const char* key);    
    LIB_LINKAGE static void*            getRawValue(const char* key);
    
    LIB_LINKAGE static s64              getInteger(const char* key, s64 defaultValue = 0ll);
    LIB_LINKAGE static f64              getFloat(const char* key, f64 defaultValue = 0.0f);
    LIB_LINKAGE static const char*      getString(const char* key, const char* defaultValue = "");
    LIB_LINKAGE static bool             getBoolean(const char* key, bool defaultValue = false);
};
}