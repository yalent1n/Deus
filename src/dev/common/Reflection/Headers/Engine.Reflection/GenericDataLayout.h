#pragma once
#include "Reflection.h"

namespace rw::GD
{
struct Layout;

struct LayoutField
{
    u32 nameHash;
    u32 offset;
    Layout* layoutPtr;
};

struct Layout
{
    u32 nameHash;
    u32 fieldCount;
    LayoutField* fields;
};

struct SortByFieldOffset
{
    inline bool operator()(const FieldInfoData& lhs, const FieldInfoData& rhs)
    {
        return lhs.fieldOffset < rhs.fieldOffset;
    }
};

class SerializationLayoutCache
{
public:
    FB_REFL_API  SerializationLayoutCache();
    FB_REFL_API ~SerializationLayoutCache();

    FB_REFL_API Layout* getDescriptor(const ClassInfo* cti);
    FB_REFL_API Layout* getDescriptor(const ValueTypeInfo* vti);
    FB_REFL_API Layout* getDescriptor(const EnumTypeInfo* eti);

    template<typename T>
    Layout* getDescriptor()
    {
        return getDescriptor(Type<T>());
    }

private:
    Dict<u32, Layout*> m_typeLayouts;
};

// Singleton instance for layout cache
FB_REFL_API SerializationLayoutCache& layoutCache();

// Maps the type to a GD layout.
FB_REFL_API Layout* mapGDLayout(const TypeInfoData& info, const TypeInfoData* superInfo = NO);

// Maps the type to a GD layout (and every derived class).
FB_REFL_API Layout* mapGDLayout(const ClassInfo& info);
}