#include <Engine.Reflection/ScriptEngine/vm.h>
#include <Engine.Base/Misc/Algorithm.h>
#include <Engine.Base/Misc/Interlocked.h>

#define SC_MEMORY_RESIDENT_PROGRAM_SIZE (sizeof(rw::scrProgram) + sizeof(rw::scrValue) + sizeof(rw::scrThread::Global))

namespace rw
{
struct GlobalFinder
{
    inline bool operator()(const scrThread::Global& index, u64 hash) const
    {
        return hash == index.Hash;
    }
};

inline constexpr u64 c_GtableHash = hashQuickConstExpr64("SC_Globals_9");

char* scrProgramFetchLabelAddress(scrProgram* prg, u32 labelIndex)
{
    char* labels = ((char*)prg + prg->LabelHeapOffset);
    u32* offsets = (u32*)((char*)prg + prg->LabelOffset);
    return labels + offsets[labelIndex];
}

void* scrProgramFetchGlobalAddress(scrProgram* prg, u64 hash)
{
    if(!prg->GlobalCount)
        return NO;

    scrThread::Global* indices = (scrThread::Global*) ((char*)prg + prg->GlobalHeapOffset);
    scrThread::Global* indicesEnd = indices + prg->GlobalCount;
    char* values = ((char*)prg + prg->GlobalOffset);
    scrThread::Global* index = algorithm::binary_search_i(indices, indicesEnd, hash, GlobalFinder());
    return (index != indicesEnd) ? values + index->Offset : NO;
}

struct mmHeapEntry
{
    u8* Block;
    size_t SizeOfBlock;
    mmHeapEntry* PointerToNextAlloc;
};

struct mmHeap
{
    u8* Start;
    u8* AllocationPoint;
    mmHeapEntry* TailEntry;
};

bool scrThread::Init(size_t heapSize)
{
    // if(!CreateHeapObject(&HeapInfo, heapSize))
    //     return false;

    if(!LoadedProgram)
    {
        //LoadedProgram = (scrProgram*) HeapAlloc(&HeapInfo, SC_MEMORY_RESIDENT_PROGRAM_SIZE);
        LoadedProgram->LabelHeapOffset = 0;
        LoadedProgram->LabelOffset = 0;
        LoadedProgram->LabelCount = 0;
        LoadedProgram->GlobalHeapOffset = 0; // TODO
        LoadedProgram->GlobalOffset = 0; // TODO
        LoadedProgram->GlobalCount = 1;
        LoadedProgram->TextSegmentOffset = 0;
    }

    return true;
}

// PURPOSE:
//  Tests if a program is compiled by 
//  matching the first allocation's size and address.
bool scrThread::IsProgramCompiled() const
{
    // mmHeapEntry* prgAllocation = HeapInfo.Link;
    // if(!prgAllocation)
    //     return false;

    // if(prgAllocation->SizeOfBlock != SC_MEMORY_RESIDENT_PROGRAM_SIZE)
    //     return false;

    // if(prgAllocation->Block != HeapInfo.Start)
    //     return false;

    // return true;
}

scrValue* scrThread::FetchGlobal(u64 hash)
{
    return (scrValue*) scrProgramFetchGlobalAddress(LoadedProgram, hash);
}

const scrValue* scrThread::FetchGlobal(u64 hash) const
{
    return (const scrValue*) scrProgramFetchGlobalAddress(LoadedProgram, hash);
}

void scrThread::RegisterGlobal(u64 hash, const scrValue& value)
{
    
}

#define Load8 (u8(*(pc++)))
#define Load16 ((pc+=2), *(u16*)(pc-2))
#define Load24 ((pc+=3), *(u32*)(pc-4) & 0xFFFFFFu)
#define Load32 ((pc+=4), *(u32*)(pc-4))
#define Load64 ((pc+=8), *(u64*)(pc-8))

template<typename AT, typename BT>
u64 cmp_bits(AT a, BT b)
{
    u64 bits = 0;
    if(a == b)
        bits |= FLAG_EQUALS;
    if(a < b)
        bits |= FLAG_LESS;
    if(a > b)
        bits |= FLAG_GREATER;

    return bits;
}

// 1b: Load Argument (ldArg)
// (text?) 1/2/3/4/8b: Loadable Literal
// (registry?) 0b: Registry slot in Load Argument
// (global?) 8b: Hash
template<typename T>
__forceinline T scrFetchAny(u8*& pc, scrThread& thread)
{
    u32 ldArg = Load8;
    u32 ldStrg = (ldArg >> 4) & 0xf;
    ldArg &= 0xf;

    switch(ldStrg)
    {
        case DS_TEXT:
            switch(ldArg) 
            {
                case 1: return (T)Load8;
                case 2: return (T)Load16;
                case 3: return (T)Load24;
                case 4: return (T)Load32;
                case 8: return (T)Load64;
            }
        case DS_SLOTS:
            return *(T*)&thread.Registers[ldArg].Any;
        case DS_TABLE:
            u64 gHash = Load64;
            return *(T*)scrProgramFetchGlobalAddress(thread.LoadedProgram, gHash);
    }
        
    __builtin_trap();
}

#define SCR_LOAD(Var, PT, VT) \
PT Var = scrFetchAny<PT>(pc, *this)

#define SCR_LOAD2(PT, VT) \
SCR_LOAD(a, PT, VT); \
SCR_LOAD(b, PT, VT)

#define SCR_CMP() \
u64 bits = cmp_bits(a, b); \
Registers[_SR4].ULong = bits

#define SCR_IMATH(PT, VT, o) \
PT bits = a o b; \
Registers[_R0].VT = bits

#define SCR_ILMATH(PT, VT, o) \
PT bits = o (a, b); \
Registers[_R0].VT = bits

#define SCR_MATH(bs, VT, o) \
if(Load8) { SCR_IMATH(u##bs, U##VT, o); } else { SCR_IMATH(s##bs, VT, o); }

#define SCR_FMATH(bs, VT, o) \
SCR_IMATH(f##bs, VT, o)

#define SCR_LMATH(bs, VT, o) \
if(Load8) { SCR_ILMATH(u##bs, U##VT, o); } else { SCR_ILMATH(s##bs, VT, o); }

// NOTE: Dst first, Src second
#define SCR_MOV(bs) \
SCR_LOAD2(void*,); \
*(u##bs*)b = *(u##bs*)a;

#define SCR_LMOV(bs) \
SCR_LOAD2(void*,); \
lock_write(*(u##bs*)b, *(u##bs*)a);

// NOTE: PC has to be stored after full INSN for correct return
#define SCR_JMP(c) \
Pushr(&Registers[0]); \
StoreFp(); \
u32 textPos = Load32; \
StorePc(); \
if(c) pc = (scrInsn*) textSegment->Start + textPos;

inline constexpr size_t SC_THREAD_STACK_SIZE = (sizeof(scrValue) * SC_MAX_STACK_SIZE) * SC_MAX_CALL_DEPTH;

#define CASE(x) case x: {
#define ENDCASE break; }
#define DISPATCH_INSN goto _DISPATCH

void scrThread::Run(scrSegment* textSegment)
{
    // Setup system registers -- start
    Registers[_SR3].Any = (scrAny) ((char*)LoadedProgram + LoadedProgram->GlobalOffset);

    scrValue* sp = ((scrValue*) HeapAllocGlobal(SC_THREAD_STACK_SIZE)) + SC_MAX_STACK_SIZE;
    StoreSp();
    StoreFp();
    
    scrInsn* pc = (scrInsn*) textSegment->Start;
    if(!pc)
        return;
        
    StorePc();
    scrInsn* endPc = pc + textSegment->Length;
    // -- end

    for(;pc < endPc;)
    {
_DISPATCH:
        switch(Load8)
        {
            default:
                break;

            CASE(OP_NOP) DISPATCH_INSN; ENDCASE

            CASE(OP_ICMP8) SCR_LOAD2(s8, Byte); SCR_CMP(); DISPATCH_INSN; ENDCASE
            CASE(OP_UICMP8) SCR_LOAD2(u8, UByte); SCR_CMP(); DISPATCH_INSN; ENDCASE
            CASE(OP_AND8) SCR_LOAD2(u8, UByte); SCR_IMATH(u8, UByte, &); DISPATCH_INSN; ENDCASE
            CASE(OP_OR8) SCR_LOAD2(u8, UByte); SCR_IMATH(u8, UByte, |); DISPATCH_INSN; ENDCASE
            CASE(OP_XOR8) SCR_LOAD2(u8, UByte); SCR_IMATH(u8, UByte, ^); DISPATCH_INSN; ENDCASE
            CASE(OP_NOT8) DISPATCH_INSN; ENDCASE

            CASE(OP_ICMP16) SCR_LOAD2(s16, Short); SCR_CMP(); DISPATCH_INSN; ENDCASE
            CASE(OP_UICMP16) SCR_LOAD2(u16, UShort); SCR_CMP(); DISPATCH_INSN; ENDCASE
            CASE(OP_AND16) SCR_LOAD2(u16, UShort); SCR_IMATH(u16, UShort, &); DISPATCH_INSN; ENDCASE
            CASE(OP_OR16) SCR_LOAD2(u16, UShort); SCR_IMATH(u16, UShort, |); DISPATCH_INSN; ENDCASE
            CASE(OP_XOR16) SCR_LOAD2(u16, UShort); SCR_IMATH(u16, UShort, ^); DISPATCH_INSN; ENDCASE
            CASE(OP_NOT16) DISPATCH_INSN; ENDCASE

            CASE(OP_ICMP32) SCR_LOAD2(s32, Int); SCR_CMP(); DISPATCH_INSN; ENDCASE
            CASE(OP_UICMP32) SCR_LOAD2(u32, UInt); SCR_CMP(); DISPATCH_INSN; ENDCASE
            CASE(OP_AND32) SCR_LOAD2(u32, UInt); SCR_IMATH(u32, UInt, &); DISPATCH_INSN; ENDCASE
            CASE(OP_OR32) SCR_LOAD2(u32, UInt); SCR_IMATH(u32, UInt, |); DISPATCH_INSN; ENDCASE
            CASE(OP_XOR32) SCR_LOAD2(u32, UInt); SCR_IMATH(u32, UInt, ^); DISPATCH_INSN; ENDCASE
            CASE(OP_NOT32) DISPATCH_INSN; ENDCASE

            CASE(OP_ICMP64) SCR_LOAD2(s64, Long); SCR_CMP(); DISPATCH_INSN; ENDCASE
            CASE(OP_UICMP64) SCR_LOAD2(u64, ULong); SCR_CMP(); DISPATCH_INSN; ENDCASE
            CASE(OP_AND64) SCR_LOAD2(u64, ULong); SCR_IMATH(u64, ULong, &); DISPATCH_INSN; ENDCASE
            CASE(OP_OR64) SCR_LOAD2(u64, ULong); SCR_IMATH(u64, ULong, |); DISPATCH_INSN; ENDCASE
            CASE(OP_XOR64) SCR_LOAD2(u64, ULong); SCR_IMATH(u64, ULong, ^); DISPATCH_INSN; ENDCASE
            CASE(OP_NOT64) DISPATCH_INSN; ENDCASE

            CASE(OP_IADD8) SCR_LOAD2(s8, Byte); SCR_MATH(8, Byte, +); DISPATCH_INSN; ENDCASE
            CASE(OP_ISUB8) SCR_LOAD2(s8, Byte); SCR_MATH(8, Byte, -); DISPATCH_INSN; ENDCASE
            CASE(OP_IMUL8) SCR_LOAD2(s8, Byte); SCR_MATH(8, Byte, *); DISPATCH_INSN; ENDCASE
            CASE(OP_IDIV8) SCR_LOAD2(s8, Byte); SCR_MATH(8, Byte, /); DISPATCH_INSN; ENDCASE

            CASE(OP_IADD16) SCR_LOAD2(s16, Short); SCR_MATH(16, Short, +); DISPATCH_INSN; ENDCASE
            CASE(OP_ISUB16) SCR_LOAD2(s16, Short); SCR_MATH(16, Short, -); DISPATCH_INSN; ENDCASE
            CASE(OP_IMUL16) SCR_LOAD2(s16, Short); SCR_MATH(16, Short, *); DISPATCH_INSN; ENDCASE
            CASE(OP_IDIV16) SCR_LOAD2(s16, Short); SCR_MATH(16, Short, /); DISPATCH_INSN; ENDCASE

            CASE(OP_IADD32) SCR_LOAD2(s32, Int); SCR_MATH(32, Int, +); DISPATCH_INSN; ENDCASE
            CASE(OP_ISUB32) SCR_LOAD2(s32, Int); SCR_MATH(32, Int, -); DISPATCH_INSN; ENDCASE
            CASE(OP_IMUL32) SCR_LOAD2(s32, Int); SCR_MATH(32, Int, *); DISPATCH_INSN; ENDCASE
            CASE(OP_IDIV32) SCR_LOAD2(s32, Int); SCR_MATH(32, Int, /); DISPATCH_INSN; ENDCASE
            CASE(OP_ILADD32) SCR_LOAD2(s32, Int); SCR_LMATH(32, Int, lock_add); DISPATCH_INSN; ENDCASE
            CASE(OP_ILSUB32) SCR_LOAD2(s32, Int); SCR_LMATH(32, Int, lock_sub); DISPATCH_INSN; ENDCASE

            CASE(OP_IADD64) SCR_LOAD2(s64, Long); SCR_MATH(64, Long, +); DISPATCH_INSN; ENDCASE
            CASE(OP_ISUB64) SCR_LOAD2(s64, Long); SCR_MATH(64, Long, -); DISPATCH_INSN; ENDCASE
            CASE(OP_IMUL64) SCR_LOAD2(s64, Long); SCR_MATH(64, Long, *); DISPATCH_INSN; ENDCASE
            CASE(OP_IDIV64) SCR_LOAD2(s64, Long); SCR_MATH(64, Long, /); DISPATCH_INSN; ENDCASE
            CASE(OP_ILADD64) SCR_LOAD2(s64, Long); SCR_LMATH(64, Long, lock_add); DISPATCH_INSN; ENDCASE
            CASE(OP_ILSUB64) SCR_LOAD2(s64, Long); SCR_LMATH(64, Long, lock_sub); DISPATCH_INSN; ENDCASE

            CASE(OP_FADD32) SCR_LOAD2(f32, Float); SCR_FMATH(32, Float, +); DISPATCH_INSN; ENDCASE
            CASE(OP_FSUB32) SCR_LOAD2(f32, Float); SCR_FMATH(32, Float, -); DISPATCH_INSN; ENDCASE
            CASE(OP_FMUL32) SCR_LOAD2(f32, Float); SCR_FMATH(32, Float, *); DISPATCH_INSN; ENDCASE
            CASE(OP_FDIV32) SCR_LOAD2(f32, Float); SCR_FMATH(32, Float, /); DISPATCH_INSN; ENDCASE

            CASE(OP_FADD64) SCR_LOAD2(f64, Double); SCR_FMATH(64, Double, +); DISPATCH_INSN; ENDCASE
            CASE(OP_FSUB64) SCR_LOAD2(f64, Double); SCR_FMATH(64, Double, -); DISPATCH_INSN; ENDCASE
            CASE(OP_FMUL64) SCR_LOAD2(f64, Double); SCR_FMATH(64, Double, *); DISPATCH_INSN; ENDCASE
            CASE(OP_FDIV64) SCR_LOAD2(f64, Double); SCR_FMATH(64, Double, /); DISPATCH_INSN; ENDCASE

            /* [STACK FRAME]
            * _______________________________
            *|   Locals   |    Parameters   | <begin>
            *|     ...   fp    C, B, A      |
            *|______________________________|
            */
            CASE(OP_CALL)
                u64 hash = Load64;
                u8 localCount = Load8;
                scrValue* val = FetchGlobal(hash);
                
                // Push the return slot address
                Pushr(&Registers[0]);
                
                StoreFp();
                sp -= localCount;
                StoreSp();

                scrCommandInfo procInfo;
                procInfo.Thread = this;
                val->Command(procInfo);

                Popr(); // Pop the return slot address
                LoadFp();
                StoreSp();
            DISPATCH_INSN;
            ENDCASE
            CASE(OP_ENTER)
                u8 localCount = Load8;
                Pushr(&Registers[0]);
                StoreFp();
                sp -= localCount; 
                StoreSp();
                DISPATCH_INSN;
            ENDCASE
            CASE(OP_LEAVE) 
                Popr(); 
                LoadFp(); 
                StoreSp();
                LoadPc(); 
                DISPATCH_INSN; 
            ENDCASE

            CASE(OP_MOV8) SCR_MOV(8); DISPATCH_INSN; ENDCASE
            CASE(OP_MOV16) SCR_MOV(16); DISPATCH_INSN; ENDCASE
            CASE(OP_MOV32) SCR_MOV(32); DISPATCH_INSN; ENDCASE
            CASE(OP_MOV64) SCR_MOV(64); DISPATCH_INSN; ENDCASE

            CASE(OP_LMOV16) SCR_LMOV(16); DISPATCH_INSN; ENDCASE
            CASE(OP_LMOV32) SCR_LMOV(32); DISPATCH_INSN; ENDCASE
            CASE(OP_LMOV64) SCR_LMOV(64); DISPATCH_INSN; ENDCASE
            
            CASE(OP_JMP) SCR_JMP(true); DISPATCH_INSN; ENDCASE
            CASE(OP_JE) SCR_JMP(_Retl(_SR4) & FLAG_EQUALS) DISPATCH_INSN; ENDCASE
            CASE(OP_JNE) SCR_JMP(!(_Retl(_SR4) & FLAG_EQUALS)); DISPATCH_INSN; ENDCASE
            CASE(OP_JG) SCR_JMP(_Retl(_SR4) & FLAG_GREATER); DISPATCH_INSN; ENDCASE
            CASE(OP_JGE) SCR_JMP(_Retl(_SR4) & (FLAG_GREATER | FLAG_EQUALS)); DISPATCH_INSN; ENDCASE
            CASE(OP_JL) SCR_JMP(_Retl(_SR4) & FLAG_LESS); DISPATCH_INSN; ENDCASE
            CASE(OP_JLE) SCR_JMP(_Retl(_SR4) & (FLAG_LESS | FLAG_EQUALS)); DISPATCH_INSN; ENDCASE

            CASE(OP_BLK)
                s32 brkCount = (s32) Popi();
                //void* block = HeapAlloc(&HeapInfo, brkCount);
                //Pusha((scrAny)block);
                DISPATCH_INSN;
            ENDCASE
            CASE(OP_RETBLK)
                // void* block = (void*) Popa();
                // HeapFree(&HeapInfo, block);
                DISPATCH_INSN;
            ENDCASE
            CASE(OP_LOAD)
                scrValue* eAddr = (scrValue*) Popa();
                Pushv(*eAddr);
                DISPATCH_INSN;
            ENDCASE
            CASE(OP_STORE)
                scrValue* eAddr = (scrValue*) Popa();
                *eAddr = Popv();
                DISPATCH_INSN;
            ENDCASE

            CASE(OP_PUSH) 
                scrAny eV = scrFetchAny<scrAny>(pc, *this); 
                Pusha(eV); 
                DISPATCH_INSN; 
            ENDCASE
            CASE(OP_POP) 
                Registers[_R0].Any = Popa(); 
                DISPATCH_INSN; 
            ENDCASE
            
            CASE(OP_CLS)
                sp = NO;
                pc = endPc;
                for(int i = 0; i < SC_MAX_USER_REGISTERS; i++)
                    Registers[i].Any = NO;
            ENDCASE
        }
    }

    HeapFreeGlobal(sp);
}
}