#include <Engine.Reflection/NativeTypeRegistry.h>

namespace rw
{
inline constexpr u32 c_maxClassDepth = 16;

struct TypeInfoComparer
{
inline bool operator()(const TypeInfo* a, const TypeInfo* b) const
{
    if(a->getTypeCategory() != b->getTypeCategory())
    {
        return a->getTypeCategory() < b->getTypeCategory();
    }

    if(a->getTypeCategory() != kTypeCategory_Class && b->getTypeCategory() != kTypeCategory_Class)
    {
        return false;
    }

    const ClassInfo* classA = ((const ClassInfo*)a)->getBaseType();
    const ClassInfo* classB = ((const ClassInfo*)b)->getBaseType();

    fixed_vector<const ClassInfo*, c_maxClassDepth> subClassesA;
    subClassesA.push_back(nullptr);
    while(classA != classA->getBaseType())
    {
        subClassesA.push_front(classA);
        classA = classA->getBaseType();
    }

    fixed_vector<const ClassInfo*, c_maxClassDepth> subClassesB;
    subClassesB.push_back(nullptr);
    while(classB != classB->getBaseType())
    {
        subClassesB.push_front(classB);
        classB = classA->getBaseType();
    }

    while(true)
    {
        classA = subClassesA.front();
        classB = subClassesB.front();

        if(classA != classB)
            break;

        subClassesA.pop_front();
        subClassesB.pop_front();
        if(subClassesA.empty() || subClassesB.empty())
            break;
    }

    if(!classA) {
        return true;
    }
    
    if(!classB)
        return false;

    if(classA != classB)
    {
        return subClassesA.size() <= subClassesB.size();
    }

    return true;
}
};

NativeTypeRegistry::NativeTypeRegistry()
{
    onInit();
}

NativeTypeRegistry::~NativeTypeRegistry()
{
    onShutdown();
}

void NativeTypeRegistry::onInit()
{
    u32 runtimeId = 0;
    TypeInfo* typeInfo = TypeInfo::s_firstTypeInfo;
    while(typeInfo)
    {
        typeInfo->m_runtimeId = runtimeId++;
        addType(typeInfo);
        typeInfo = typeInfo->m_nextTypeInfo;
    }

    ClassId classId = 0;

    algorithm::insertion_sort(m_typeInfos.begin(), m_typeInfos.end(), TypeInfoComparer());
    for(TypeInfo* ti : m_typeInfos)
    {
        if(ti->getTypeCategory() == kTypeCategory_Class)
        {
            ClassInfo* classInfo = (ClassInfo*)ti;
            
            for(ClassInfo* ci = classInfo; ci != ci->getBaseType(); ci = const_cast<ClassInfo*>(ci->getBaseType()))
            {
                ci->getClassInfoData()->lastSubclassId = classId;
            }

            ClassInfoData* classData = classInfo->getClassInfoData();
            classData->classId = classId++;
            addClass(classInfo);
        }
    }

    // for(const ClassInfo* ci : m_classInfos)
    // {
    //     ELOGF("%s.%s :: clsId=%u, lastClsId=%u", ci->getModule()->getName(), ci->getName(), ci->getClassId(), ci->getLastSubclassId());
    // }
}

void NativeTypeRegistry::onShutdown()
{

}

void NativeTypeRegistry::addType(const TypeInfo* typeInfo)
{
    if(!typeInfo)
    {
        return;
    }

    if(m_typeInfoMap.find(typeInfo->getName()) != m_typeInfoMap.end())
    {
        return;
    }

    m_typeInfoMap.insert(typeInfo->getName(), const_cast<TypeInfo*>(typeInfo));
    m_typeInfoGuidMap.insert(typeInfo->getGuid(), const_cast<TypeInfo*>(typeInfo));
    m_typeInfos.push_back(const_cast<TypeInfo*>(typeInfo));
}

TypeInfo* NativeTypeRegistry::findType(const Guid& typeGuid)
{
    if(typeGuid == c_emptyGuid)
    {
        return nullptr;
    }

    auto it = m_typeInfoGuidMap.find(typeGuid);
    if(it != m_typeInfoGuidMap.end())
        return it->second;

    return nullptr;
}

TypeInfo* NativeTypeRegistry::findType(const char* typeName)
{
    if(isnil(typeName))
    {
        return nullptr;
    }

    auto it = m_typeInfoMap.find(typeName);
    if(it != m_typeInfoMap.end())
        return it->second;
        
    return nullptr;
}

TypeInfo* NativeTypeRegistry::findQualifiedType(const char* qualifiedTypeName)
{
    return nullptr;
}

void NativeTypeRegistry::addClass(const ClassInfo* classInfo)
{
    if(!classInfo)
    {
        return;
    }

    if(m_classInfoMap.find(classInfo->getName()) != m_classInfoMap.end())
    {
        return;
    }

    m_classInfoMap.insert(classInfo->getName(), const_cast<ClassInfo*>(classInfo));
    m_typeInfoGuidMap.insert(classInfo->getGuid(), const_cast<ClassInfo*>(classInfo));
    m_classInfos.push_back(const_cast<ClassInfo*>(classInfo));
}

ClassInfo* NativeTypeRegistry::findClass(const Guid& classGuid)
{
    if(classGuid == c_emptyGuid)
    {
        return nullptr;
    }

    auto it = m_classInfoGuidMap.find(classGuid);
    if(it != m_classInfoGuidMap.end())
        return it->second;
        
    return nullptr;
}

ClassInfo* NativeTypeRegistry::findClass(const char* className)
{
    if(isnil(className))
    {
        return nullptr;
    }

    auto it = m_classInfoMap.find(className);
    if(it != m_classInfoMap.end())
        return it->second;
        
    return nullptr;
}

ClassInfo* NativeTypeRegistry::findQualifiedClass(const char* qualifiedClassName)
{
    return nullptr;
}

ClassInfo* NativeTypeRegistry::findClass(ClassId classId)
{
    if(classId >= m_classInfos.size())
    {
        return nullptr;
    }

    return m_classInfos[classId];
}
}