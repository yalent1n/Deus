#pragma once
#include "Reflection.h"

namespace rw
{
struct EbxPartitionFieldDescriptor
{
    u32             fieldNameHash;
    MemberInfoFlags flags;
    u16             fieldType;
    u32             fieldOffset;

    /*
    ** During serialization this is the offset into original instance.
    */
    u32             secondaryOffset;
};

struct EbxPartitionTypeDescriptor
{
    u32 typeNameHash;
    u32 layoutDescriptor;
    u8 fieldCount;
    u8 alignment;
    MemberInfoFlags flags;
    u16 instanceSize;

    /*
    ** During serialization this is the size of original instance.
    */
    u16 secondarySize;
};

struct EbxPartitionImportEntry
{
    Guid partitionGuid;
    Guid instanceGuid;
};

// ---------
// Shared types
// ---------

struct EbxSharedTypeDescriptorKey
{
    u8 hash[16];
};

struct HashEbxSharedTypeDescriptorKey
{
    inline unsigned operator()(const EbxSharedTypeDescriptorKey& key)
    {
        return DJB(key.hash, sizeof key.hash);
    }
};

struct EbxSharedTypeDescriptor
{
    EbxSharedTypeDescriptorKey key;
    EbxPartitionTypeDescriptor typeDescriptor;
};
}