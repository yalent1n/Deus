#pragma once
#include "Bundle.h"
#include <Engine.Base/Crypto/SHA.h>
#include <Engine.Base/Crypto/ECC.h>
#include <Engine.Base/Crypto/HKDF.h>
#include <Engine.Base/String/CString.h>
#include <Engine.Base/Crypto/Guid.h>

#define EKVM_ADDRESS_LENGTH 32
#define EKVM_KEY_LENGTH 32
#define XCHG_AEAD_LENGTH 16

namespace Deus::Exchange
{
using cString = rw::String;

enum SUBSYTEM : rw::s8
{
    LOGICAL_SUBSYSTEM = 0x65,
    PHYSICAL_SUBSYSTEM = 0x66,
    SUBSYSTEM_COUNT = 2
};

typedef ECC_key Key; // Key to accessing others from the network, like a password.
typedef rw::SHA256 Hash, ID;
typedef unsigned long long U64;

// Address of a device in the network
#pragma pack(push, 1)
struct Address {
    enum addrtype : rw::u8 { YEVEN = 3, YODD = 4 };
    addrtype t;
    ECC_field d;

    template<typename _Curve = secp256k1Curve>
    bool decode(ECC_curve<_Curve>& c) const {
        c.inf = false;
        c.x = d;
        return ecc_p256_decode<_Curve>(c.y, c.x, t==YODD);
    }

    template<typename _Curve = secp256k1Curve>
    bool encode(const ECC_curve<_Curve>& c) {
        if(c.inf)
            return false;

        d = c.x;
        t = (c.y.d[0] & 1) ? YODD : YEVEN;
        return true;
    }
};
#pragma pack(pop)

// aead=Assigned Exchange Authentication Data
struct AEAD
{
    char auth_data[XCHG_AEAD_LENGTH];
};

enum IDClass : char
{
    P2P_CLASS, // Peer to Peer
    MLTS_CLASS, // Peer to Multiple (multi-send)
    MLTR_CLASS, // Multiple to Peer (multi-recv)
    LEDGER_CLASS, // Multiple to multiple
};

struct Event
{
    int index;
    U64 vout;
};

/*
    [header]
    [input hashes]
    [output hashes]
    [aead list]
    [exchange events list]
*/
struct Junction
{
    Hash sig_hash;
    int pin_count;
    int pout_count;
    int epush_count;
    int epull_count;
    int exchange_count;
};

FB_NETWORK_API bool random(void*, size_t);

struct HandshakeParams
{
    Address addr;
    Key key;
};

FB_NETWORK_API bool handshake(const HandshakeParams* params, ID* xid);

struct BuildParams
{
    IDClass xchg_class;
};

struct BuildParamsP2P : BuildParams
{
    // This shold be the source's key.
    Key k;
    Address dst_addr;
};

// for MLTR and MLTS
struct BuildParamsMULTI : BuildParams
{
    union { Address src; Address dst; } addr;
    Key k;
    union { Hash ihash; Hash ohash; } root;
};

struct BuildParamsLEDGER : BuildParams
{
    Hash iroot;
    Hash oroot;
};

struct VerifyParams
{
    IDClass xchg_class;
};

struct VerifyParamsP2P : VerifyParams
{
    Key k;
    Address dst_addr;
};

struct VerifyParamsMLTS : VerifyParams
{
    Key k;
    Hash oroot;
};

struct VerifyParamsMLTR : VerifyParams
{
    // TODO(?) : This should be a consensus secret among the input peers.
    Key k;
    Hash iroot;
    Address dst_addr;
};

struct VerifyParamsUNI : VerifyParams
{
    Hash ihash;
    Hash ohash;
};

FB_NETWORK_API bool xchg(const BuildParams*, ID* xid, AEAD* aead);
FB_NETWORK_API bool verify(const VerifyParams*, const ID* xid, const AEAD* aead);

#define SealParams \
const Hash*        xt_hash, \
const cString&          xchg_func, \
const void*             xchg_data, \
long long               xchg_data_len

FB_NETWORK_API Hash seal(SealParams);

struct Tree
{
    eastl::vector<eastl::vector<Hash>> levels;
};

FB_NETWORK_API void merkleBuild(Tree& tree, const eastl::vector<Hash>& hashes);
FB_NETWORK_API bool merkleVerify(const Tree& tree, int index, const Hash& hash);
FB_NETWORK_API const Hash& merkleRoot(const Tree& tree);

struct Map
{
    struct Point
    {
        ECC_curve<secp256k1Curve> pub;
        ECC_key k;
    };

    struct Events
    { 
        eastl::vector<Event> pulls; 
        eastl::vector<Event> pushes;
    };

    eastl::vector<Point> inputs;
    eastl::vector<Point> outputs;
    eastl::vector<ID> ids;
    eastl::vector<AEAD> aeads;
    eastl::hash_map<ID, Events> events;
};

FB_NETWORK_API int Input(Map& map, bool generate_keys = YES);
FB_NETWORK_API int AddInputEvent(Map& map, const ID& xchg_id, const Event& iev);

FB_NETWORK_API int Output(Map& map, bool generate_keys = YES);
FB_NETWORK_API int AddOutputEvent(Map& map, const ID& xchg_id, const Event& oev);

FB_NETWORK_API int layoutP2PEvents(Map& map, int input_index, U64 val, int output_index);
FB_NETWORK_API int layoutMLTSEvents(Map& map, int input_index, int output_index_base, const eastl::vector<U64>& vals);
FB_NETWORK_API int layoutMLTREvents(Map& map, int output_index, int input_index_base, const eastl::vector<U64>& vals);
FB_NETWORK_API int layoutLEDGEREvents(Map& map, int input_index_base, const eastl::vector<U64>& ivals, int output_index_base, const eastl::vector<U64>& ovals);

FB_NETWORK_API rw::SharedBuffer* Build(const Map& map);
}