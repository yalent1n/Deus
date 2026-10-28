#pragma once
#include "instructions.h"
#include "mmudefines.h"
#include <Engine.Base/Memory/Map.h>
#include <Engine.Base/HashMap.h>

namespace rw
{
inline constexpr size_t c_TranslationTableEntryCount = _MMU_LOGICAL_PAGE_SIZE / 8;
inline constexpr size_t c_PageTableEntryCount = _MMU_EFFECTIVE_PAGE_SIZE / 8;
inline constexpr size_t c_LogicalPageSize = _MMU_LOGICAL_PAGE_SIZE;

typedef void* mm_laddr;
typedef const void* mm_laddr_r;

struct mmTranslationTableEntry
{
    u64 eFrameNumber : _MMU_EFFECTIVE_LEVELS * _MMU_EFFECTIVE_WIDTH;

    /* NOTE: There can be a max amount of 28 flags. */
    u64 Present : 1;
    u64 Read : 1;
    u64 Write : 1;
    u64 Execute : 1;
    u64 SystemReserved : 1;
};

typedef void* mm_eaddr;
typedef const void* mm_eaddr_r;

struct mmPageTableEntry
{
    u64 lFrameNumber : _MMU_LOGICAL_LEVELS * _MMU_LOGICAL_WIDTH;
    u64 EffectiveFrameEnd : _MMU_EFFECTIVE_LEVELS * _MMU_EFFECTIVE_WIDTH;
};

union mmTranslationTable
{
    mmTranslationTableEntry Translations[c_TranslationTableEntryCount];
    mmTranslationTable* Tables[c_TranslationTableEntryCount];
};

union mmPageTable
{
    mmPageTableEntry Pages[c_PageTableEntryCount];
    mmPageTable* Tables[c_PageTableEntryCount];
};

/*
    PURPOSE: 
    
    Handles the script VM's address space
    using 2-level adressing. This MMU works on
    a 42 bit address space, which looks like 
    in the following diagram:

    - 14 BIT PAGE OFFSET
    - 14 BIT TRANSLATION ENTRY
    - 14 BIT TRANSLATION GROUP ENTRY.

    A TT entry has a weak reference to a PT entry, 
    so fetching an Effective Address 
    from a Logical Address works vice-versa. 
*/
class mmSystem
{
    static SC_API mmPageTable GlobalDescriptorTable;
    static SC_API mmTranslationTable TranslationGroupTable;

public:
    // Returns the master page table which maps effective frames to logical ones.
    static SC_API mmPageTable*             PageTable();

    // Translates an effective address to a logical one if it's set.
    static SC_API mm_laddr                 lTranslate(mm_eaddr_r eAddress);
    
    // Returns the master translation table which stores logical frame properties.
    static SC_API mmTranslationTable*      TranslationTable();

    // Translates a logical address to an effecitve one if it's set.
    static SC_API mm_eaddr                 eTranslate(mm_laddr_r lAddress);

    // Creates the mapping between logical and effective frames.
    static SC_API int                      Set(mm_laddr lBaseAddress, mm_eaddr eBaseAddress, size_t RegionSize);

    // Erases that mapping from both tables.
    static SC_API int                      UnSet(mm_laddr lBaseAddress);
};

SC_API mm_eaddr mmCopyData(mm_laddr DstAddress, mm_laddr_r SrcAddress, size_t Size);
SC_API mm_eaddr mmMoveData(mm_laddr DstAddress, mm_laddr_r SrcAddress, size_t Size);
SC_API mm_eaddr mmFillData(mm_laddr DstAddress, u8 Byte, size_t Size);
SC_API mm_eaddr mmZeroData(mm_laddr DstAddress, size_t Size);
}