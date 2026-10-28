#pragma once
#include "../Bundle.h"

#define SC_API FB_REFL_API

#define scrHash hashQuick64

#define scrLiteralConst(arg) u8((u32(DS_TEXT) << 4) | u32(arg))
#define scrLiteralRslot(i) u8((u32(DS_SLOTS) << 4) | u32(i))

namespace rw
{
enum scrInsnDataSource : u8
{
    DS_TEXT,
    DS_SLOTS,
    DS_TABLE
};

typedef u8 scrInsn;

// NOTE:
// L prefix means Interlocked.
// XCHG suffix means Interlocked Exchange.
enum : scrInsn
{
    OP_NOP,

    // Math 8-bit operations
    OP_ICMP8,
    OP_UICMP8,
    OP_AND8,
    OP_OR8,
    OP_XOR8,
    OP_NOT8,

    // Math 16-bit operations
    OP_ICMP16,
    OP_UICMP16,
    OP_AND16,
    OP_OR16,
    OP_XOR16,
    OP_NOT16,

    // Math 32-bit operations
    OP_ICMP32,
    OP_UICMP32,
    OP_FCMP32,
    OP_AND32,
    OP_OR32,
    OP_XOR32,
    OP_NOT32,
    
    // Math 64-bit operations
    OP_ICMP64,
    OP_UICMP64,
    OP_FCMP64,
    OP_AND64,
    OP_OR64,
    OP_XOR64,
    OP_NOT64,

    // Integer 8-bit operations
    OP_IADD8,
    OP_ISUB8,
    OP_IMUL8,
    OP_IDIV8,

    // Integer 16-bit operations
    OP_IADD16,
    OP_ISUB16,
    OP_IMUL16,
    OP_IDIV16,

    // Integer 32-bit operations
    OP_IADD32,
    OP_ISUB32,
    OP_IMUL32,
    OP_IDIV32,
    OP_ILADD32,
    OP_ILSUB32,

    // Integer 64-bit operations
    OP_IADD64,
    OP_ISUB64,
    OP_IMUL64,
    OP_IDIV64,
    OP_ILADD64,
    OP_ILSUB64,

    // Floating-point 32-bit operations
    OP_FADD32,
    OP_FSUB32,
    OP_FMUL32,
    OP_FDIV32,

    // Floating-point 64-bit operations
    OP_FADD64,
    OP_FSUB64,
    OP_FMUL64,
    OP_FDIV64,

    // Callee-Caller instructions
    OP_CALL,
    OP_ENTER,
    OP_LEAVE,

    // Move instructions
    OP_MOV8,
    OP_MOV16,
    OP_MOV32,
    OP_MOV64,
    OP_LMOV16,
    OP_LMOV32,
    OP_LMOV64,
    
    // Jump instructions
    OP_JMP,
    OP_JE,
    OP_JNE,
    OP_JG,
    OP_JGE,
    OP_JL,
    OP_JLE,

    // Memory instructions
    OP_BLK, // heap alloc
    OP_RETBLK, // heap free
    OP_LOAD, // load value from the loaded address
    OP_STORE,

    OP_PUSH,
    OP_POP,

    OP_CLS // CLear Script
};
}