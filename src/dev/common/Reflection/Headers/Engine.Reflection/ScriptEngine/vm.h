#pragma once
#include "program.h"
#include "threaddefines.h"

#define SC_LOAD_STACKFRAME scrValue* sp; \
do { \
    scrValue* Registers = info.Thread->Registers; \
    LoadFp(); \
} while(false)

namespace rw
{
struct scrThread;

typedef u32 scrThreadId;

struct scrThreadTable
{
    scrThread* Threads[SC_MAX_THREAD_COUNT];
};

CompileTimeAssertSize(scrThreadTable, _MMU_LOGICAL_PAGE_SIZE);

struct scrThreadLocalStorage
{
    scrValue Locals[SC_MAX_THREAD_LOCAL_COUNT];
};

CompileTimeAssertSize(scrThreadLocalStorage, _MMU_EFFECTIVE_PAGE_SIZE);

struct scrThread
{
    enum State
    {
        RUNNING, BLOCKED, ABORTED, EXITED
    };

    struct Label
    {
        u32 Offset;
    };

    struct Global
    {
        int Type;
        int Offset;
        u64 Hash;
    };

    State State;
    scrThreadId Id;

    scrProgram* LoadedProgram;
    scrValue Registers[SC_MAX_REGISTERS];

    FB_REFL_API bool                    Init(size_t heapSize);

    FB_REFL_API bool                    IsProgramCompiled() const;

    FB_REFL_API scrValue*               FetchGlobal(u64 hash);
    FB_REFL_API const scrValue*         FetchGlobal(u64 hash) const;
    FB_REFL_API void                    RegisterGlobal(u64 hash, const scrValue& value);

    FB_REFL_API void                    Run(scrProgram::Segment* textSegment);
};
}