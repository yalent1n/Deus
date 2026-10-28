#pragma once
#include "mm.h"
#include <Engine.Base/Buffer/Adapter.h>

namespace rw
{
inline constexpr u32        cIMAGE_BASE = 0x50000000;
inline constexpr u32        cIMAGE_STRING_SEED = 0x6DEAD6BEEF6;

/*
    PURPOSE:
    Stores the resource size in page counts to store larger
    integers more compact.
*/
struct scrDataLayout
{
    u32 PageShift : 6;

    u32 PageCount : 8;
    u32 Page2Count : 6;
    u32 Page4Count : 4;
    u32 Page8Count : 2;
    u32 Page16Count : 1;
    u32 Page32Count : 1;
    u32 Page64Count : 1;
    u32 Page128Count : 1;
    u32 Page256Count : 1;
    u32 Page512Count : 1;

    // NOTE: Total size can be configured with the c_LogicalPageSize constant
    StreamSize GetTotalSize() const
    {
        StreamSize pageSize = c_LogicalPageSize << PageShift;
        return (
            (Page512Count << 9) + 
            (Page256Count << 8) + 
            (Page128Count << 7) + 
            (Page64Count << 6) + 
            (Page32Count << 5) + 
            (Page16Count << 4) + 
            (Page8Count << 3) + 
            (Page4Count << 2) + 
            (Page2Count << 1) + 
            PageCount) * pageSize;
    }
};

SC_API scrDataLayout ComputeDataLayout(size_t Size);

struct scrResource
{
    s32 NameOffset;
    s32 DataOffset;
    
    s32 Size;
    scrDataLayout DataLayout;
};

/*
    PURPOSE:
    Stores a logical pointer to a resource descriptor.
*/
struct scrResourceHandle
{
    mm_laddr Address;
};

struct scrImage
{
    s32         Signature;
    s32         Checksum;

    s32         StringBlockAddress;
    s32         StringBlockSize;

    s32         ResourceAddress;
    s32         NumberOfResources;

    s32         ImportAddress;
    s32         NumberOfImports;

    s32         ExportAddress;
    s32         NumberOfExports;
};

class scrImageBuilder
{
public:
    scrImageBuilder() = default;
    ~scrImageBuilder() = default;

    // Builds the script image.
    SC_API scrImage* Build();

    SC_API void AddResource(const char* Name, mm_eaddr_r Address, size_t Size);
    SC_API void DeleteResource(const char* ResourceName);

    SC_API void AddImport(scrResourceHandle Handle);
    SC_API void DeleteImport(scrResourceHandle Handle);

    SC_API void AddExport(scrResourceHandle Handle);
    SC_API void DeleteExport(scrResourceHandle Handle);

private:
    s32 AddString(const char* Str);

private:
    eastl::hash_map<s32, s32> m_StringTable;
    AllocatingBuffer m_StringBuffer;
};
}