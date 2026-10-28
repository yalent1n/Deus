#pragma once
#include "registerdefines.h"
#include "stackdefines.h"
#include "value.h"
#include "mm.h"
#include <Engine.Base/Buffer/Adapter.h>

namespace rw
{
/*
    STRUCTURE:

    [scrProgram Header]
    [array scrProgram::Segment]
    [array scrThread::Label]
    [array scrThread::Global]
    [heap Global]
    [heap Label]
*/
struct scrProgram
{
    struct Segment
    {
        /* Pointer to the segment beginning */
        u32             Offset;

        /* Size of the segment */
        u32             Length;
    };

    /* Pointer to a heap where labels are stored (C-strings) */
    u32                 LabelHeapOffset;

    /* Pointer to a list of offsets into the label heap */
    u32                 LabelOffset;

    /* Count of labels (TODO: either jump labels or any string in program) */
    u32                 LabelCount;

    /* Pointer to a heap where globals' values are stored */
    u32                 GlobalHeapOffset;

    /* Pointer to a list of scrThread::Global */
    u32                 GlobalOffset;

    /* Count of globals in the program */
    u32                 GlobalCount;

    /* Pointer to an scrSegment struct which contains the program code location */
    u32                 TextSegmentOffset;
};

SC_API char* scrProgramFetchLabelAddress(scrProgram* prg, u32 idx);
SC_API void* scrProgramFetchGlobalAddress(scrProgram* prg, u64 hash);

struct scrThread { struct Global; };

class scrProgramBuilder
{
public:
    SC_API scrProgramBuilder();
    SC_API ~scrProgramBuilder();

    /* Adds a label and returns the index of it */
    SC_API u32 AddLabel(const char* label);

    /* Adds a global and returns the index of it */
    SC_API u32 AddGlobal(u64 hash, const scrValue& value);

    ////////////////////////////
    // Here bellow are instruction generating functions (for writing bytecode)
    ////////////////////////////

    /* Enters a stack frame, with N locals */
    SC_API void Enter(u8 localCount = 0);

    /* Leaves a stack frame */
    SC_API void Leave();

    /* Writes a literal to the text */
    SC_API void Literal(scrInsnDataSource dataSource, u32 dataArg, const void* dataValue);

    /* Calls a native function */
    SC_API void Call(u64 hash, u8 localCount = 0);

    /* Performs a math operation (integer/floating-point) */
    SC_API void Math(scrInsn op, const void* aNum, const void* bNum, bool sign = NO);

    /* Writes a const literal into the bytecode (inlines) */
    template<typename T>
    inline void ConstData(const T& c)
    {
        Literal(DS_TEXT, sizeof(c), &c);
    }

    /* Writes a slot (register) literal into the bytecode */
    inline void SlotData(u8 slot)
    {
        EASSERT(slot < SC_MAX_USER_REGISTERS, "Unable to inline system register in scrProgramBuilder");
        Literal(DS_SLOTS, u32(slot), nullptr);
    }

    // TODO: GlobalData function

    /* Marks the end of script */
    SC_API void ClearScript();

    /* Exports the program  */
    SC_API scrProgram* Export(AllocatingBuffer& exportBuffer);

private:
    eastl::vector<scrValue> m_Globals;
    eastl::vector<scrThread::Global> m_GlobalIndices;
};
}