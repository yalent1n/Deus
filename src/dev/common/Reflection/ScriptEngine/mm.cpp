#include <Engine.Reflection/ScriptEngine/mm.h>

namespace rw
{
mmPageTable mmSystem::GlobalDescriptorTable = {};
mmTranslationTable mmSystem::TranslationGroupTable = {};

// TGT -> TT
mmTranslationTableEntry* MmFetchTranslationEntry(mm_laddr_r lAddr, mmTranslationTable& GroupTable)
{
    s16 Index = TTE1(lAddr),
        GroupIndex = TTE0(lAddr);

    mmTranslationTable *Table = NULL;
    
    Table = GroupTable.Tables[GroupIndex];
    mmTranslationTableEntry* Entry = Table->Translations + Index;
    if(!Entry->Present)
    {
        return NULL;
    }

    return Entry;
}

mmTranslationTableEntry* MmObtainTranslationEntry(mm_laddr lAddr, mmTranslationTable& GroupTable)
{
    s16 Index = TTE1(lAddr),
        GroupIndex = TTE0(lAddr);

    mmTranslationTable *Table = NULL;
    
    Table = GroupTable.Tables[GroupIndex];
    if(!Table)
    {
        Table = (mmTranslationTable*) sysMem.SystemAlloc(sizeof (mmTranslationTable));
        memset(Table, 0, sizeof (mmTranslationTable));
        GroupTable.Tables[GroupIndex] = Table;
    }

    return Table->Translations + Index;
}

// (PML4) -> PDPT -> PDT -> PT
mmPageTableEntry* MmFetchPageEntry(mm_eaddr_r eAddr, mmPageTable& GlobalDescriptorTable)
{
    s16 Index = PTE3(eAddr),
        TableIndex = PTE2(eAddr),
        DirectoryTableIndex = PTE1(eAddr),
        DirectoryGroupTableIndex = PTE0(eAddr);

    mmPageTable *Table = NULL, *DirectoryTable = NULL, *DirectoryGroupTable = NULL;

    DirectoryGroupTable = GlobalDescriptorTable.Tables[DirectoryGroupTableIndex];
    if(!DirectoryGroupTable)
        return NULL;

    DirectoryTable = DirectoryGroupTable->Tables[DirectoryTableIndex];
    if(!DirectoryTable)
        return NULL;

    Table = DirectoryTable->Tables[TableIndex];
    if(!Table)
        return NULL;

    return Table->Pages + Index;
}

mmPageTableEntry* MmObtainPageEntry(mm_eaddr eAddr, mmPageTable& GlobalDescriptorTable)
{
    s16 Index = PTE3(eAddr),
        TableIndex = PTE2(eAddr),
        DirectoryTableIndex = PTE1(eAddr),
        DirectoryGroupTableIndex = PTE0(eAddr);

    mmPageTable *Table = NULL, *DirectoryTable = NULL, *DirectoryGroupTable = NULL;

    DirectoryGroupTable = GlobalDescriptorTable.Tables[DirectoryGroupTableIndex];
    if(!DirectoryGroupTable)
    {
        DirectoryGroupTable = (mmPageTable*) sysMem.SystemAlloc(sizeof (mmPageTable));
        memset(DirectoryGroupTable, 0, sizeof (mmPageTable));
        GlobalDescriptorTable.Tables[DirectoryGroupTableIndex] = DirectoryGroupTable;
    }

    DirectoryTable = DirectoryGroupTable->Tables[DirectoryTableIndex];
    if(!DirectoryTable)
    {
        DirectoryTable = (mmPageTable*) sysMem.SystemAlloc(sizeof (mmPageTable));
        memset(DirectoryTable, 0, sizeof (mmPageTable));
        DirectoryGroupTable->Tables[DirectoryTableIndex] = DirectoryTable;
    }

    Table = DirectoryTable->Tables[TableIndex];
    if(!Table)
    {
        Table = (mmPageTable*) sysMem.SystemAlloc(sizeof (mmPageTable));
        memset(Table, 0, sizeof (mmPageTable));
        DirectoryTable->Tables[TableIndex] = Table;
    }

    return Table->Pages + Index;
}

mmPageTable* mmSystem::PageTable()
{
    return &GlobalDescriptorTable;
}

mm_laddr mmSystem::lTranslate(mm_eaddr_r eAddress)
{
    s16 lPageOffset = s16(u64(eAddress) & _MASK(_MMU_LOGICAL_OFFSET));

    mmPageTableEntry* Entry = MmFetchPageEntry(eAddress, GlobalDescriptorTable);
    if(!Entry)
        return NULL;

    mm_laddr lAddr = (mm_laddr) ((Entry->lFrameNumber << _MMU_LOGICAL_OFFSET) | u64(lPageOffset));
    return lAddr;
}

mmTranslationTable* mmSystem::TranslationTable()
{
    return &TranslationGroupTable;
}

mm_eaddr mmSystem::eTranslate(mm_laddr_r lAddress)
{
    s16 ePageOffset = s16(u64(lAddress) & _MASK(_MMU_EFFECTIVE_OFFSET));

    mmTranslationTableEntry* Entry = MmFetchTranslationEntry(lAddress, TranslationGroupTable);
    if(!Entry)
        return NULL;

    mm_eaddr eAddr = (mm_eaddr) ((Entry->eFrameNumber << _MMU_EFFECTIVE_OFFSET) | u64(ePageOffset));
    return eAddr;
}

#define PageRoundUp(N) RoundUp(N, _MMU_LOGICAL_PAGE_SIZE)

int mmSystem::Set(mm_laddr lBaseAddress, mm_eaddr eBaseAddress, size_t RegionSize)
{
    char* lAddr = (char*) lBaseAddress;
    char* lAddrEnd = (char*) lBaseAddress + RegionSize;
    lAddrEnd = (char*) PageRoundUp(u64(lAddrEnd));

    char* eAddr = (char*) eBaseAddress;
    u64 EffectiveFrameEnd = u64(eBaseAddress) + RegionSize;
    EffectiveFrameEnd = PageRoundUp(EffectiveFrameEnd) >> _MMU_EFFECTIVE_OFFSET;

    while(lAddr < lAddrEnd)
    {
        mmTranslationTableEntry* TranslationEntry = MmObtainTranslationEntry(lAddr, TranslationGroupTable);
        if(!TranslationEntry)
        {
            return NO;
        }

        mmPageTableEntry* PageEntry = MmObtainPageEntry(eAddr, GlobalDescriptorTable);
        if(!PageEntry)
        {
            return NO;
        }

        TranslationEntry->eFrameNumber = u64(eAddr) >> _MMU_EFFECTIVE_OFFSET;
        TranslationEntry->Present = YES;
        TranslationEntry->Read = YES;
        TranslationEntry->Write = YES;
        TranslationEntry->Execute = NO;
        TranslationEntry->SystemReserved = NO;

        PageEntry->lFrameNumber = u64(lAddr) >> _MMU_LOGICAL_OFFSET;
        PageEntry->EffectiveFrameEnd = EffectiveFrameEnd;

        lAddr += _MMU_LOGICAL_PAGE_SIZE;
        eAddr += _MMU_LOGICAL_PAGE_SIZE;
    }

    return YES;
}

int mmSystem::UnSet(mm_laddr lBaseAddress)
{
    mmTranslationTableEntry* TranslationEntry = MmFetchTranslationEntry(lBaseAddress, TranslationGroupTable);
    if(!TranslationEntry)
    {
        return NO;
    }

    char* eBaseAddress = (char*) (TranslationEntry->eFrameNumber << _MMU_EFFECTIVE_OFFSET);
    mmPageTableEntry* PageEntry = MmFetchPageEntry(eBaseAddress, GlobalDescriptorTable);
    if(!PageEntry)
    {
        return NO;
    }

    char* eAddr = eBaseAddress;
    char* eAddrEnd = (char*) (PageEntry->EffectiveFrameEnd << _MMU_EFFECTIVE_OFFSET);
    size_t RegionSize = (size_t) (eAddrEnd - eBaseAddress);

    char* lAddr = (char*) lBaseAddress;
    char* lAddrEnd = (char*) lBaseAddress + RegionSize;

    while(lAddr < lAddrEnd)
    {
        TranslationEntry = MmFetchTranslationEntry(lAddr, TranslationGroupTable);
        *(u64*)TranslationEntry = 0;

        PageEntry = MmFetchPageEntry(eAddr, GlobalDescriptorTable);
        *(u64*)PageEntry = 0;

        lAddr += _MMU_LOGICAL_PAGE_SIZE;
        eAddr += _MMU_LOGICAL_PAGE_SIZE;
    }

    return YES;
}

mm_eaddr mmCopyData(mm_laddr DstAddress, mm_laddr_r SrcAddress, size_t Size)
{
    DstAddress = mmSystem::eTranslate(DstAddress);
    if(!DstAddress)
    {
        return NULL;
    }

    SrcAddress = mmSystem::eTranslate(SrcAddress);
    if(!SrcAddress)
    {
        return NULL;
    }

    return memcpy(DstAddress, SrcAddress, Size);
}

mm_eaddr mmMoveData(mm_laddr DstAddress, mm_laddr_r SrcAddress, size_t Size)
{
    DstAddress = mmSystem::eTranslate(DstAddress);
    if(!DstAddress)
    {
        return NULL;
    }

    SrcAddress = mmSystem::eTranslate(SrcAddress);
    if(!SrcAddress)
    {
        return NULL;
    }

    return memmove(DstAddress, SrcAddress, Size);
}

mm_eaddr mmFillData(mm_laddr DstAddress, u8 Byte, size_t Size)
{
    DstAddress = mmSystem::eTranslate(DstAddress);
    if(!DstAddress)
    {
        return NULL;
    }

    return memset(DstAddress, (int) Byte, Size);
}

mm_eaddr mmZeroData(mm_laddr DstAddress, size_t Size)
{
    DstAddress = mmSystem::eTranslate(DstAddress);
    if(!DstAddress)
    {
        return NULL;
    }

    return memset(DstAddress, 0, Size);
}
}