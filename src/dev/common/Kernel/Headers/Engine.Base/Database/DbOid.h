#pragma once
#include "../Types.h"

namespace rw
{
    class StringEditor;
    
    class DbOid
    {
        friend class DbObjectElement;

    public:
        LIB_LINKAGE DbOid();

        static DbOid c_zero;
        
        static LIB_LINKAGE void init();
        LIB_LINKAGE void generate();

        LIB_LINKAGE void toString(StringEditor& str) const;
        LIB_LINKAGE bool parse(const char* str);
        LIB_LINKAGE bool parse(const char* str, size_t len);
        
        LIB_LINKAGE u32 session() const;
        LIB_LINKAGE u32 serial() const;
    private:
        u32 id[3] { 0 };
    };
}
