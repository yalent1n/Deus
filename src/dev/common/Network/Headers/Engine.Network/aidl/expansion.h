#pragma once
#include "frame.h"
#include "error.h"
#include <Engine.Core/diag/Environment.h>

namespace rw
{

struct aidl_neighbour
{
    const char*                 hostname;
    ether_addr                  addr;
    ECC_curve<secp256k1Curve>   pub;
};

typedef eastl::hash_map<u32, aidl_neighbour> aidl_neighbour_table;

enum Aidl_Expansion_Flags : u32 {
    Aidl_Expansion_Estabilished = 0x01,
    Aidl_Expansion_Estabilished_Multiple = 0x02,
};

struct Aidl_Expansion_State {
    ECC_key k;
    aidl_neighbour self;
    aidl_neighbour_table table;
    u32 flags;
};

AIDL_API void _AidlExpConfigTag(aidl_expansion_tag* tag, u8 op, const aidl_neighbour* node);

AIDL_API int _Aidl_open_door (Aidl_Expansion_State* state, aidl_expansion_tag* tag);
AIDL_API int _Aidl_ring_doorbell (Aidl_Expansion_State* state, aidl_expansion_tag* tag);
AIDL_API int _Aidl_expansion_init_state (Aidl_Expansion_State* state);
AIDL_API int aidl_expansion_set (Aidl_Expansion_State* state, const ether_addr& addr, const char* hostname = NO, const Deus::Exchange::Address* pub = NO);
AIDL_API aidl_neighbour* aidl_expansion_get_neighbour (const Aidl_Expansion_State* state, const ether_addr& addr);
AIDL_API int aidl_expansion_remove_neighbour (Aidl_Expansion_State* state, const ether_addr& addr);
}