#include <Engine.Reflection/GenericDataLayout.h>

namespace rw::GD
{
SerializationLayoutCache::SerializationLayoutCache()
{}

SerializationLayoutCache::~SerializationLayoutCache()
{}

Layout* SerializationLayoutCache::getDescriptor(const ClassInfo* cti)
{
    EASSERT(cti, "Null class type pointer");
    const auto it = m_typeLayouts.find(cti->getNameHash());
    if(it != m_typeLayouts.end())
        return it->second;

    Layout* layout = mapGDLayout(*cti);
    m_typeLayouts.insert(layout->nameHash, layout);
    return layout;
}

Layout* SerializationLayoutCache::getDescriptor(const ValueTypeInfo* vti)
{
    EASSERT(vti, "Null value type pointer");
    const auto it = m_typeLayouts.find(vti->getNameHash());
    if(it != m_typeLayouts.end())
        return it->second;

    Layout* layout = mapGDLayout(*vti->getTypeInfoData());
    m_typeLayouts.insert(layout->nameHash, layout);
    return layout;
}

Layout* SerializationLayoutCache::getDescriptor(const EnumTypeInfo* eti)
{
    EASSERT(eti, "Null enum type pointer");
    const auto it = m_typeLayouts.find(eti->getNameHash());
    if(it != m_typeLayouts.end())
        return it->second;

    Layout* layout = mapGDLayout(*eti->getTypeInfoData());
    m_typeLayouts.insert(layout->nameHash, layout);
    return layout;
}

SerializationLayoutCache& layoutCache()
{
    static SerializationLayoutCache s_cache;
    
    // RUN_ONCE(s_cache);
    // s_cache = new SerializationLayoutCache();

    // RAN_ONCE = YES;
    return s_cache;
}

Layout* mapGDLayout(const TypeInfoData& info, const TypeInfoData* superInfo)
{
    u32 super = superInfo && info.fieldInfos[0].fieldOffset != OffsetToFirstField::value;

    Layout* layout = new Layout();
    layout->nameHash = info.member.nameHash;
    layout->fields = nullptr;
    layout->fieldCount = 0;

    u32 totalFieldCount = info.fieldCount;
    if(super)
    {
        ++totalFieldCount;
    }

    if(totalFieldCount)
    {
        LayoutField* fields = new LayoutField[totalFieldCount];

        u32 firstFieldIndex = 0;
        if(super)
        {
            LayoutField& superField = fields[0];
            superField.nameHash = REFL_HASH_CONSTEXPR("$");
            superField.offset = superInfo->size;
            superField.layoutPtr = nullptr;
            ++firstFieldIndex;
        }

        for(u32 i = 0; i < info.fieldCount; i++)
        {
            const FieldInfoData& field = info.fieldInfos[i];
            LayoutField& layoutField = fields[firstFieldIndex + i];
            layoutField.nameHash = field.member.nameHash;
            layoutField.offset = field.fieldOffset;
            layoutField.layoutPtr = nullptr;
        }

        layout->fields = fields;
        layout->fieldCount = totalFieldCount;
    }

    return layout;
}

Layout* mapGDLayout(const ClassInfo& info)
{
    const ClassInfo* cti = &info;
    
    Layout* layout = nullptr;
    Layout* layoutPtr = nullptr;
    while(cti != cti->getBaseType())
    {
        const ClassInfo* baseInfo = cti->getBaseType();
        
        Layout* nextLayoutPtr = mapGDLayout(*cti->getTypeInfoData(), baseInfo->getTypeInfoData());
        if(!nextLayoutPtr)
        {
            return nullptr;
        }
        
        if(layoutPtr)
        {
            layoutPtr->fields[0].layoutPtr = nextLayoutPtr;
        }

        layoutPtr = nextLayoutPtr;
        cti = cti->getBaseType();

        if(!layout)
        {
            layout = layoutPtr;
        }
    }

    return layout;
}
}