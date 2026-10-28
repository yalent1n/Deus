#pragma once
#include "Bundle.h"
#include "TypeInfo.h"
#include "ClassInfo.h"

namespace rw
{
class TypeRegistry
{
public:
    virtual ~TypeRegistry() = default;
    
    virtual void onInit() = 0;
    virtual void onShutdown() = 0;

    virtual void addType(const TypeInfo* typeInfo) = 0;
    virtual TypeInfo* findType(const Guid& typeGuid) = 0;
    virtual TypeInfo* findType(const char* typeName) = 0;
    virtual TypeInfo* findQualifiedType(const char* qualifiedTypeName) = 0;
    
    virtual void addClass(const ClassInfo* classInfo) = 0;
    virtual TypeInfo* findClass(const Guid& typeGuid) = 0;
    virtual ClassInfo* findClass(const char* typeName) = 0;
    virtual ClassInfo* findQualifiedClass(const char* qualifiedTypeName) = 0;
};
}