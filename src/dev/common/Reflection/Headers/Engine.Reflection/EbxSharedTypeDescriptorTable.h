#pragma once
#include "EbxPartition.h"
#include <map>

namespace rw
{
class EbxSharedTypeDescriptorTable
{
public:
    typedef std::multimap<EbxSharedTypeDescriptorKey, EbxSharedTypeDescriptor*> TypeMap_t;

public:
    FB_REFL_API  EbxSharedTypeDescriptorTable();
    FB_REFL_API ~EbxSharedTypeDescriptorTable();

private:
    TypeMap_t m_typeMap;
};
}