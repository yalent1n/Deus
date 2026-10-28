#pragma once

// User registries
#define _R0 0
#define _R1 1
#define _R2 2
#define _R3 3
#define _R4 4
#define _R5 5
#define _R6 6
#define _R7 7
#define _R8 8
#define _R9 9
#define _R10 10

// System registries
#define _SR0 11 /* Program Counter/PC */
#define _SR1 12 /* Stack-top pointer/SP */
#define _SR2 13 /* Frame Pointer/FP */
#define _SR3 14 /* Global table/GT */
#define _SR4 15 /* Flags/FL */

#define StorePc() Registers[_SR0].Program = pc
#define StoreSp() Registers[_SR1].Reference = sp + 1
#define StoreFp() Registers[_SR2].Reference = sp + 1

#define LoadPc() pc = Registers[_SR0].Program
#define LoadSp() sp = Registers[_SR1].Reference - 1
#define LoadFp() sp = Registers[_SR2].Reference - 1

namespace rw
{
inline constexpr size_t SC_MAX_USER_REGISTERS       = 11;
inline constexpr size_t SC_MAX_SYSTEM_REGISTERS     = 5;
inline constexpr size_t SC_MAX_REGISTERS            = SC_MAX_USER_REGISTERS + SC_MAX_SYSTEM_REGISTERS;

enum : u64
{
    FLAG_EQUALS = _BIT(1),
    FLAG_GREATER = _BIT(2),
    FLAG_LESS = _BIT(3),
};
}