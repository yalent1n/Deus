#pragma once

#define SC_MAX_STACK_SIZE 256
#define SC_MAX_CALL_DEPTH 16

#define Pushb(b) (--sp)->UByte = (b)
#define Pushs(s) (--sp)->UShort = (s)
#define Pushi(i) (--sp)->UInt = (i)
#define Pushl(l) (--sp)->ULong = (l)
#define Pushstr(str) (--sp)->String = (str)
#define Pushr(ref) (--sp)->Reference = (ref)
#define Pusha(v) (--sp)->Any = (v)
#define Pushv(v) *(--sp) = (v)

#define Popb() ((sp++)->UByte)
#define Pops() ((sp++)->UShort)
#define Popi() ((sp++)->UInt)
#define Popl() ((sp++)->ULong)
#define Popstr() ((sp++)->String)
#define Popr() ((sp++)->Reference)
#define Popa() ((sp++)->Any)
#define Popv() (*(sp++))

#define _Retb(slot) (sp)->Reference[slot].UByte
#define _Rets(slot) (sp)->Reference[slot].UShort
#define _Reti(slot) (sp)->Reference[slot].UInt
#define _Retl(slot) (sp)->Reference[slot].ULong
#define _Retstr(slot) (sp)->Reference[slot].String
#define _Retr(slot) (sp)->Reference[slot].Reference
#define _Reta(slot) (sp)->Reference[slot].Any

#define Retb() _Retb(0)
#define Rets() _Rets(0)
#define Reti() _Reti(0)
#define Retl() _Retl(0)
#define Retstr() _Retstr(0)
#define Retr() _Retr(0)
#define Reta() _Reta(0)