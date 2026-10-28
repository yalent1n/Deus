#pragma once
#include "TypeRegistry.h"

namespace rw
{
class NativeTypeRegistry : public TypeRegistry
{
public:
    typedef Vec<TypeInfo*> TypeInfos_t;
    typedef Dict<const char*, TypeInfo*> TypeInfoNameMap_t;
    typedef Dict<Guid, TypeInfo*> TypeInfoGuidMap_t;
    typedef Vec<ClassInfo*> ClassInfos_t;
    typedef Dict<const char*, ClassInfo*> ClassInfoNameMap_t;
    typedef Dict<Guid, ClassInfo*> ClassInfoGuidMap_t;

public:
    FB_REFL_API NativeTypeRegistry();
    FB_REFL_API ~NativeTypeRegistry() override;

    FB_REFL_API void onInit() override;
    FB_REFL_API void onShutdown() override;

    FB_REFL_API void addType(const TypeInfo* typeInfo) override;
    FB_REFL_API TypeInfo* findType(const Guid& typeGuid) override;
    FB_REFL_API TypeInfo* findType(const char* typeName) override;
    FB_REFL_API TypeInfo* findQualifiedType(const char* qualifiedTypeName) override;

    FB_REFL_API void addClass(const ClassInfo* classInfo) override;
    FB_REFL_API ClassInfo* findClass(const Guid& classGuid) override;
    FB_REFL_API ClassInfo* findClass(const char* className) override;
    FB_REFL_API ClassInfo* findQualifiedClass(const char* qualifiedClassName) override;
    FB_REFL_API ClassInfo* findClass(ClassId classId);

private:
    TypeInfos_t m_typeInfos;
    TypeInfoNameMap_t m_typeInfoMap;
    TypeInfoGuidMap_t m_typeInfoGuidMap;
    ClassInfos_t m_classInfos;
    ClassInfoNameMap_t m_classInfoMap;
    ClassInfoGuidMap_t m_classInfoGuidMap;
};

FB_REFL_API NativeTypeRegistry* g_nativeTypeRegistry;
}