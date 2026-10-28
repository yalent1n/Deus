#pragma once
#include "TypeInfo.h"
#include "ArrayTypeInfo.h"

namespace rw
{
typedef void(*CreateRefFun_t)(void* placement);
typedef void(*SetRefFun_t)(void* placement, void* oP);
typedef void(*DestroyRefFun_t)(void* placement);
typedef void(*ResolveRefFun_t)(void* placement);
typedef u32 ClassId;

inline constexpr ClassId c_invalidClassId = ClassId(-1); 

class ClassInfo;

struct ClassInfoData
{
    TypeInfoData basic;
    const ClassInfo* baseType;
    CreateRefFun_t createRef;
    SetRefFun_t setRef;
    DestroyRefFun_t destroyRef;
    ResolveRefFun_t resolveRef;
    ClassId classId;
    ClassId lastSubclassId;
};

class ClassInfo : public TypeInfo
{
public:
    FB_REFL_API         ClassInfo(const ClassInfoData* infoData);
    FB_REFL_API virtual ~ClassInfo();

    ImplementInfoDataGetter(ClassInfoData)

    const ClassInfo* getBaseType() const
        { return getClassInfoData()->baseType; }
    
    ClassId getClassId() const
        { return getClassInfoData()->classId; }
    
    ClassId getLastSubclassId() const
        { return getClassInfoData()->lastSubclassId; }
    
    ClassId getBaseClassId() const
        { return getBaseType() ? getBaseType()->getClassId() : c_invalidClassId; }

    FB_REFL_API bool isSubclassOf(const ClassInfo* classInfo) const;

    template<class T>
    bool isSubclassOf() const { return isSubclassOf(Type<T>()); }

    FB_REFL_API void createRef(void* placement);
    FB_REFL_API void setRef(void* placement, void* oP);
    FB_REFL_API void destroyRef(void* placement);
    FB_REFL_API void resolveRef(void* placement);

    SkeletonNode* toSkel(const void* data, const SkelConversionContext* context) const override;
    void toString(const void* data, const StringConversionParams* params, StringEditor& result) const override;
};

template<typename T>
const ClassInfo* Class()
    { return &_TypeHelper< _TypeConv<T>::type >::typedata::c_typeInfo; }

template<typename T>
const ClassInfo* Class(const T&)
    { return &_TypeHelper< _TypeConv<T>::type >::typedata::c_typeInfo; }

template<typename T>
const ClassInfo* Class(const T*)
    { return &_TypeHelper< _TypeConv<T>::type >::typedata::c_typeInfo; }

template<typename T>
const ClassInfo* Class(const SmartHandle<T>&)
    { return &_TypeHelper<T>::typedata::c_typeInfo; }
}