#include <Engine.Network/aidl/expansion.h>

namespace rw
{
void _AidlExpConfigTag(aidl_expansion_tag* tag, u8 op, const aidl_neighbour* node)
{
    size_t len = strlen(node->hostname);

    tag->type = AIDL_NODE_EXPANSION_TLV;
    tag->length = sizeof(tl) + 1 + sizeof(Deus::Exchange::Address) + len;
    tag->op = op;
    tag->pub.encode(node->pub);

    memcpy(&tag->host[0], node->hostname, len);
    tag->host[len] = 0;
}

int _Aidl_open_door(Aidl_Expansion_State* state, aidl_expansion_tag* tag) 
{
    if(!tag || !state)
        return DL_ERR_PARAMETER_NULL;

    _AidlExpConfigTag(tag, AIDL_EXP_DOOR_OPENED, &state->self);
    return DL_OK;
}

int _Aidl_ring_doorbell(Aidl_Expansion_State* state, aidl_expansion_tag* tag) 
{
    if(!tag || !state)
        return DL_ERR_PARAMETER_NULL;

    
    _AidlExpConfigTag(tag, AIDL_EXP_RING_DOORBELL, &state->self);
    return DL_OK;
}

AIDL_API int _Aidl_expansion_init_state(Aidl_Expansion_State* state) 
{
    if(!state)
        return DL_ERR_PARAMETER_NULL;

    state->self.hostname = Environment::getHostId();
    Deus::Exchange::random(&state->k, sizeof(state->k));
    ecc_p256_mul<secp256k1Curve>(state->self.pub.x, state->self.pub.y, state->k);
    return DL_OK;
}

int aidl_expansion_set (Aidl_Expansion_State* state, const ether_addr& addr, const char* hostname, const Deus::Exchange::Address* pub)
{
    if(!state)
        return DL_ERR_PARAMETER_NULL;

    aidl_neighbour& n = state->table[addr.in4.sin_addr.s_addr];
    n.addr = addr;

    if(hostname && n.hostname)
    {
        delete n.hostname;
        n.hostname = hostname;
    }

    if(pub)
        pub->decode(n.pub);

    return DL_OK;
}

aidl_neighbour* aidl_expansion_get_neighbour (const Aidl_Expansion_State* state, const ether_addr& addr)
{
    aidl_neighbour_table::iterator i = state->table.find(addr.in4.sin_addr.s_addr);
    if(i != state->table.end()) 
        return &i->second;
    
    return nullptr;
}

int aidl_expansion_remove_neighbour (Aidl_Expansion_State* state, const ether_addr& addr)
{
    aidl_neighbour_table::iterator i = state->table.find(addr.in4.sin_addr.s_addr);
    if(i != state->table.end())  {
        state->table.erase(i);
        return DL_OK;
    }
    
    return DL_ERR_TARGET_NOT_FOUND;
}
}