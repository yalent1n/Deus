#include <Engine.Reflection/ScriptEngine/image.h>

namespace rw
{
inline constexpr size_t cMAX_DATAPAGE_COUNT = 
    (_MASK(1) * 512) + 
    (_MASK(1) * 256) + 
    (_MASK(1) * 128) + 
    (_MASK(1) * 64) + 
    (_MASK(1) * 32) + 
    (_MASK(1) * 16) +
    (_MASK(2) * 8) +
    (_MASK(4) * 4) +
    (_MASK(6) * 2) +
    (_MASK(8) * 1);

scrDataLayout ComputeDataLayout(size_t rawSize)
{
    scrDataLayout layout {};
    *(s32*)&layout = 0;
    
    size_t pageSize = c_LogicalPageSize << layout.PageShift;
    u32 pageCount = (rawSize + pageSize - 1) / pageSize;

    for(;;)
    {
        if(layout.PageShift >= 64)
            break;
        
        pageSize = c_LogicalPageSize << layout.PageShift;
        pageCount = (rawSize + pageSize - 1) / pageSize;
        if(pageCount <= cMAX_DATAPAGE_COUNT)
            break;

        layout.PageShift++;
    }

    pageSize = c_LogicalPageSize << layout.PageShift;
    pageCount = (rawSize + pageSize - 1) / pageSize;

    layout.Page512Count = pageCount / 512;
    pageCount %= 512;
    layout.Page256Count = pageCount / 256;
    pageCount %= 256;
    layout.Page128Count = pageCount / 128;
    pageCount %= 128;
    layout.Page64Count = pageCount / 64;
    pageCount %= 64;
    layout.Page32Count = pageCount / 32;
    pageCount %= 32;
    layout.Page16Count = pageCount / 16;
    pageCount %= 16;
    layout.Page8Count = pageCount / 8;
    pageCount %= 8;
    layout.Page4Count = pageCount / 4;
    pageCount %= 4;
    layout.Page2Count = pageCount / 2;
    pageCount %= 2;

    layout.PageCount = pageCount;

    return layout;
}

scrImage* Build();

void scrImageBuilder::AddResource(const char* Name, mm_eaddr_r Address, size_t Size)
{
    if(isnil(Name) || !Address || !Size)
    {
        return;
    }


}

void scrImageBuilder::DeleteResource(const char* ResourceName)
{

}

void scrImageBuilder::AddImport(scrResourceHandle Handle)
{

}

void scrImageBuilder::DeleteImport(scrResourceHandle Handle)
{

}

void scrImageBuilder::AddExport(scrResourceHandle Handle)
{

}

void scrImageBuilder::DeleteExport(scrResourceHandle Handle)
{

}

s32 scrImageBuilder::AddString(const char* Str)
{
    if(isnil(Str))
    {
        return -1;
    }

    u32 StrHash = hashQuick(Str, cIMAGE_STRING_SEED);
    if(auto Slot = m_StringTable.find_by_hash(StrHash); 
        Slot != m_StringTable.end())
    {
        return Slot->second;
    }

    s32 Offset = (s32) m_StringBuffer.getAvailableBytes();
    m_StringBuffer.encodeString(Str);
    m_StringTable.insert(s32(StrHash), Offset);

    return Offset;
}
}