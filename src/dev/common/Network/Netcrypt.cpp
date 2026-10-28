#include <Engine.Network/Netcrypt.h>
#include <Engine.Network/aidl/schedule.h>
#include <Engine.Base/Database/DbObjectBuilder.h>
#include <Engine.Base/Buffer/Adapter.h>
#include <Engine.Base/String/String.h>
#include <Engine.Core/diag/ExecutionContext.h>

#ifdef SvcCompilerMSVC
    #include <Windows.h>
    #include <bcrypt.h>
#endif

namespace Deus::Exchange
{
// Periodical Secrecy Discussion Timing
inline constexpr double c_PSDT = (13.0 * 30.0) / 5.0;

bool random(void* dst, size_t len)
{
    if(!dst || !len)
        return NO;

    NTSTATUS Status = BCryptGenRandom(NULL, (PUCHAR)dst, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return BCRYPT_SUCCESS(Status);
}

bool handshake(const HandshakeParams* params, ID* xid) {
    if(!params || !xid)
        return false;

    ECC_curve<secp256k1Curve> pub {};
    if(!params->addr.decode(pub))
        return false;

    EllipticCurveGF<secp256k1Curve> x {}, y {};
    ecc_p256_mul(x, y, params->key, pub);

    if(x.is_zero())
        return false;

    xid->calculate(&x, sizeof(x));
    return true;
}

bool xchg(const BuildParams* params, ID* xchg_id, AEAD* aead)
{
    if(!params)
        return NO;

    if(!xchg_id)
        return NO;

    if(!aead)
        return NO;

    switch(params->xchg_class)
    {
        default:
            return NO;

        case P2P_CLASS:
        {
            const BuildParamsP2P* p2p_params = (const BuildParamsP2P*) params;

            HandshakeParams xparams {};
            xparams.addr = p2p_params->dst_addr;
            xparams.key = p2p_params->k;
            
            constexpr int _SIZE = sizeof(ID) + sizeof(AEAD);
            constexpr int _ID_OFFSET = 0;
            constexpr int _AEAD_OFFSET = sizeof(ID);
            char deriv_data[_SIZE] {};

            if(!handshake(&xparams, (ID*)&deriv_data[_ID_OFFSET]))
                return NO;

            if(!random(deriv_data + _AEAD_OFFSET, sizeof(AEAD)))
                return NO;

            if(!rw::Crypto::HkdfDerive(rw::Crypto::kAlgSHA384, deriv_data, _SIZE, xchg_id, sizeof(ID), "Exchange_P2P_Logical", 20, &xparams.key, sizeof(xparams.key)))
                return NO;

            memcpy(aead, deriv_data + _AEAD_OFFSET, sizeof(AEAD));
            return YES;
        }

        case MLTS_CLASS:
        {
            const BuildParamsMULTI* multi_params = (const BuildParamsMULTI*) params;

            constexpr int _SIZE = sizeof(Hash) + sizeof(AEAD) + sizeof(Hash);
            constexpr int _OUTHASH_OFFSET = 0;
            constexpr int _AEAD_OFFSET = sizeof(Hash);
            constexpr int _SRCHASH_OFFSET = sizeof(Hash) + sizeof(AEAD);
            char deriv_data[_SIZE] {};

            memcpy(deriv_data + _OUTHASH_OFFSET, &multi_params->root.ohash, sizeof(Hash));
            ((Hash*)deriv_data + _SRCHASH_OFFSET)->calculate(&multi_params->k, sizeof(multi_params->k));
            if(!random(deriv_data + _AEAD_OFFSET, sizeof(AEAD)))
                return NO;

            if(!rw::Crypto::HkdfDerive(rw::Crypto::kAlgSHA384, deriv_data, _SIZE, xchg_id, sizeof(ID), "Exchange_MLTS_Logical", 21, &multi_params->k, sizeof(multi_params->k)))
                return NO;

            memcpy(aead, deriv_data + _AEAD_OFFSET, sizeof(AEAD));
            return YES;
        }

        case MLTR_CLASS:
        {
            const BuildParamsMULTI* multi_params = (const BuildParamsMULTI*) params;

            constexpr int _SIZE = sizeof(Hash) + sizeof(AEAD) + sizeof(Hash);
            constexpr int _INHASH_OFFSET = sizeof(Hash) + sizeof(ID);
            constexpr int _AEAD_OFFSET = sizeof(Hash);
            constexpr int _DSTHASH_OFFSET = 0;
            char deriv_data[_SIZE] {};

            memcpy(deriv_data + _INHASH_OFFSET, &multi_params->root.ihash, sizeof(Hash));
            ((Hash*)deriv_data + _DSTHASH_OFFSET)->calculate(&multi_params->addr.dst, sizeof(multi_params->addr));
            if(!random(deriv_data + _AEAD_OFFSET, sizeof(AEAD)))
                return NO;

            if(!rw::Crypto::HkdfDerive(rw::Crypto::kAlgSHA384, deriv_data, _SIZE, xchg_id, sizeof(ID), "Exchange_MLTR_Logical", 21, &multi_params->k, sizeof(multi_params->k)))
                return NO;

            memcpy(aead, deriv_data + _AEAD_OFFSET, sizeof(AEAD));
            return YES;
        }

        case LEDGER_CLASS:
        {
            const BuildParamsLEDGER* multimap_params = (const BuildParamsLEDGER*) params;

            constexpr int _SIZE = sizeof(Hash) + sizeof(AEAD) + sizeof(Hash);
            constexpr int _INHASH_OFFSET = 0;
            constexpr int _AEAD_OFFSET = sizeof(Hash);
            constexpr int _OUTHASH_OFFSET = sizeof(Hash) + sizeof(ID);
            char deriv_data[_SIZE] {};

            memcpy(deriv_data + _INHASH_OFFSET, &multimap_params->iroot, sizeof(Hash));
            memcpy(deriv_data + _OUTHASH_OFFSET, &multimap_params->oroot, sizeof(Hash));
            if(!random(deriv_data + _AEAD_OFFSET, sizeof(AEAD)))
                return NO;

            if(!rw::Crypto::HkdfDerive(rw::Crypto::kAlgSHA384, deriv_data, _SIZE, xchg_id, sizeof(ID), "Exchange_UNI_Logical", 20, deriv_data + _AEAD_OFFSET, sizeof(AEAD)))
                return NO;

            memcpy(aead, deriv_data + _AEAD_OFFSET, sizeof(AEAD));
            return YES;
        }
    }
}

bool verify(const VerifyParams* params, const ID* xid, const AEAD* aead)
{
    if(!params || !xid)
        return NO;

    switch(params->xchg_class)
    {
        default:
            return NO;

        case P2P_CLASS:
        {
            const VerifyParamsP2P* p2p_params = (const VerifyParamsP2P*) params;

            HandshakeParams xparams {};
            xparams.addr = p2p_params->dst_addr;
            xparams.key = p2p_params->k;
            
            constexpr int _SIZE = sizeof(ID) + sizeof(AEAD);
            constexpr int _ID_OFFSET = 0;
            constexpr int _AEAD_OFFSET = sizeof(ID);
            char deriv_data[_SIZE] {};
    
            if(!handshake(&xparams, (ID*)&deriv_data[_ID_OFFSET]))
                return NO;
    
            memcpy(deriv_data + _AEAD_OFFSET, aead, sizeof(AEAD));
    
            ID test_xid;
            if(!rw::Crypto::HkdfDerive(rw::Crypto::kAlgSHA384, deriv_data, _SIZE, &test_xid, sizeof(ID), "Equilibrium_P2P_Logical", 23, &xparams.key, sizeof(xparams.key)))
                return NO;
    
            return test_xid == *xid;
        }

        case MLTS_CLASS:
        {
            const VerifyParamsMLTS* multi_params = (const VerifyParamsMLTS*) params;

            constexpr int _SIZE = sizeof(Hash) + sizeof(AEAD) + sizeof(Hash);
            constexpr int _OUTHASH_OFFSET = 0;
            constexpr int _AEAD_OFFSET = sizeof(Hash);
            constexpr int _SRCHASH_OFFSET = sizeof(Hash) + sizeof(AEAD);
            char deriv_data[_SIZE] {};

            memcpy(deriv_data + _OUTHASH_OFFSET, &multi_params->oroot, sizeof(Hash));
            ((Hash*)deriv_data + _SRCHASH_OFFSET)->calculate(&multi_params->k, sizeof(multi_params->k));
            memcpy(deriv_data + _AEAD_OFFSET, aead, sizeof(AEAD));

            ID test_xid;
            if(!rw::Crypto::HkdfDerive(rw::Crypto::kAlgSHA384, deriv_data, _SIZE, &test_xid, sizeof(ID), "Equilibrium_MLTS_Logical", 24, &multi_params->k, sizeof(multi_params->k)))
                return NO;

            return test_xid == *xid;
        }

        case MLTR_CLASS:
        {
            const VerifyParamsMLTR* multi_params = (const VerifyParamsMLTR*) params;

            constexpr int _SIZE = sizeof(Hash) + sizeof(AEAD) + sizeof(Hash);
            constexpr int _DSTHASH_OFFSET = 0;
            constexpr int _AEAD_OFFSET = sizeof(Hash);
            constexpr int _INHASH_OFFSET = sizeof(Hash) + sizeof(ID);
            char deriv_data[_SIZE] {};

            memcpy(deriv_data + _INHASH_OFFSET, &multi_params->iroot, sizeof(Hash));
            ((Hash*)deriv_data + _DSTHASH_OFFSET)->calculate(&multi_params->dst_addr, sizeof(multi_params->dst_addr));
            memcpy(deriv_data + _AEAD_OFFSET, aead, sizeof(AEAD));

            ID test_xid;
            if(!rw::Crypto::HkdfDerive(rw::Crypto::kAlgSHA384, deriv_data, _SIZE, &test_xid, sizeof(ID), "Equilibrium_MLTR_Logical", 24, &multi_params->k, sizeof(multi_params->k)))
                return NO;

            return test_xid == *xid;
        }
    
        case LEDGER_CLASS:
        {
            const VerifyParamsUNI* uni_params = (const VerifyParamsUNI*) params;

            constexpr int _SIZE = sizeof(Hash) + sizeof(AEAD) + sizeof(Hash);
            constexpr int _INHASH_OFFSET = 0;
            constexpr int _AEAD_OFFSET = sizeof(Hash);
            constexpr int _OUTHASH_OFFSET = sizeof(Hash) + sizeof(ID);
            char deriv_data[_SIZE] {};

            memcpy(deriv_data + _INHASH_OFFSET, &uni_params->ihash, sizeof(Hash));
            memcpy(deriv_data + _OUTHASH_OFFSET, &uni_params->ohash, sizeof(Hash));
            memcpy(deriv_data + _AEAD_OFFSET, aead, sizeof(AEAD));

            ID test_xid;
            if(!rw::Crypto::HkdfDerive(rw::Crypto::kAlgSHA384, deriv_data, _SIZE, &test_xid, sizeof(ID), "equilibrium.uni.logical", 23, deriv_data + _AEAD_OFFSET, sizeof(AEAD)))
                return NO;

            return test_xid == *xid;
        }
    }
}

Hash seal(SealParams)
{
    rw::FixedStringBuilder<> seal_info;
    seal_info << "Equilibrium_" << xchg_func.c_str() << "_Physical";

    Hash seal_hash {};

    rw::Crypto::HkdfDerive(
        rw::Crypto::kAlgSHA512, 
        xchg_data, 
        xchg_data_len, 
        &seal_hash,
        sizeof(seal_hash),
        seal_info.c_str(),
        seal_info.size(),
        xt_hash, 
        sizeof(Hash)
    );

    return seal_hash;
}

inline Hash _HashLeaf(const Hash& s) {
    rw::AllocatingBuffer buf;
    buf.write(0x00); // leaf domain-separation prefix
    buf.write(&s.hash, sizeof(Hash));

    Hash hash;
    hash.calculate(buf.bytes(), buf.getAvailableBytes());
    return hash;
}
 
inline Hash _HashInternal(const Hash& left, const Hash& right) {
    rw::AllocatingBuffer buf;
    buf.write(0x01); // leaf domain-separation prefix
    buf.write(&left.hash, sizeof(Hash));
    buf.write(&right.hash, sizeof(Hash));

    Hash hash;
    hash.calculate(buf.bytes(), buf.getAvailableBytes());
    return hash;
}

void merkleBuild(Tree& tree, const eastl::vector<Hash>& hashes)
{
    if (hashes.empty())
        return;

    tree.levels.push_back();
    for (Hash s : hashes) 
        tree.levels[0].push_back(_HashLeaf(s));

    while (tree.levels.back().size() > 1) 
    {
        const auto& cur = tree.levels.back();
        eastl::vector<Hash> next;

        size_t i = 0;
        for (; i + 1 < cur.size(); i += 2)
            next.push_back(_HashInternal(cur[i], cur[i + 1]));

        if (i < cur.size())
            next.push_back(cur[i]); // odd one out: promote unchanged, don't duplicate

        tree.levels.push_back(next);
        next.detach();
    }
}

struct ProofStep
{
    const Hash& s;
    bool right;
};

bool _ekvm_Proof(const Tree* tree, int index, eastl::vector<ProofStep>& steps)
{
    if (index < 0 || (size_t)index >= tree->levels[0].size())
        return false;
    
    size_t idx = index;
    for (size_t level = 0; level + 1 < tree->levels.size(); ++level) {
        const auto& cur = tree->levels[level];
        bool hasSibling = (idx ^ 1) < cur.size();
        if (hasSibling) {
            size_t sibIdx = idx ^ 1;
            steps.push_back({cur[sibIdx], sibIdx > idx});
        }
        // if no sibling (odd one promoted), this node just moves up unchanged --
        // no proof step needed for that level.
        idx /= 2;
    }
    return true;
}

bool merkleVerify(const Tree& tree, int index, const Hash& hash)
{
    if(tree.levels.empty())
        return false;

    eastl::vector<ProofStep> proof;
    if(!_ekvm_Proof(&tree, index, proof))
        return false;

    rw::SHA256 running = _HashLeaf(hash);
    for (const auto& step : proof) {
        running = step.right ? _HashInternal(running, step.s)
                            : _HashInternal(step.s, running);
    }

    return memcmp(&running, &merkleRoot(tree), sizeof(Hash)) == 0;
}

const Hash& merkleRoot(const Tree& tree)
{
    return tree.levels.back().front();
}

int Input(Map& map, bool generate_keys)
{
    int index = map.inputs.size();
    Map::Point& P = map.inputs.push_back();
    if(generate_keys) {
        if(!random(&P.k, sizeof(P.k)))
            return -1;

        ecc_p256_mul<secp256k1Curve>(P.pub.x, P.pub.y, P.k);
        if(P.pub.x.is_zero())
            return -1;

        P.pub.inf = false;
    }
    
    return index;
}

int AddInputEvent(Map& map, const ID& xchg_id, const Event& iev)
{
    if(iev.index >= map.inputs.size())
        return -1;

    auto& inevents = map.events[xchg_id].pulls;
    int index = inevents.size();
    inevents.push_back(iev);
    return index;
}

int Output(Map& map, bool generate_keys)
{
    int index = map.outputs.size();
    Map::Point& P = map.outputs.push_back();
    if(generate_keys) {
        if(!random(&P.k, sizeof(P.k)))
            return -1;

        ecc_p256_mul<secp256k1Curve>(P.pub.x, P.pub.y, P.k);
        if(P.pub.x.is_zero())
            return -1;

        P.pub.inf = false;
    }

    return index;
}

int AddOutputEvent(Map& map, const ID& xchg_id, const Event& oev)
{
    if(oev.index >= map.outputs.size())
        return -1;

    auto& outevents = map.events[xchg_id].pushes;
    int index = outevents.size();
    outevents.push_back(oev);
    return index;
}

int layoutP2PEvents(Map& map, int input_index, U64 val, int output_index)
{
    if(input_index >= map.inputs.size() || output_index >= map.outputs.size())
        return -1;

    BuildParamsP2P xparams;
    xparams.xchg_class = P2P_CLASS;
    xparams.k = map.inputs[input_index].k;
    xparams.dst_addr.encode(map.outputs[output_index].pub);

    int index = map.ids.size();
    ID& xchg_id = map.ids.push_back();
    AEAD& xchg_aead = map.aeads.push_back();
    if(!xchg(&xparams, &xchg_id, &xchg_aead))
        return -1;

    auto& events = map.events[xchg_id];
    events.pulls.push_back({input_index, val});
    events.pushes.push_back({output_index, val});
    return index;
}

int layoutMLTSEvents(Map& map, int input_index, int output_index_base, const eastl::vector<U64>& vals)
{
    if(input_index >= map.inputs.size())
        return -1;

    U64 sum = 0;

    eastl::vector<Hash> ohashes;
    for(int i = 0; i < vals.size(); i++)
    {
        const Map::Point& output = map.outputs[output_index_base + i];
        ohashes.push_back().calculate(&output.pub.x, sizeof(output.pub.x));
        sum += vals[i];
    }

    Tree xoutputs;
    merkleBuild(xoutputs, ohashes);

    BuildParamsMULTI xparams {};
    xparams.xchg_class = MLTS_CLASS;
    xparams.addr.src.encode(map.inputs[input_index].pub);
    xparams.root.ohash = merkleRoot(xoutputs);

    int index = map.ids.size();
    ID& xchg_id = map.ids.push_back();
    AEAD& xchg_aead = map.aeads.push_back();
    if(!xchg(&xparams, &xchg_id, &xchg_aead))
        return -1;

    auto& events = map.events[xchg_id];
    events.pulls.push_back({input_index, sum});

    for(int i = 0; i < vals.size(); i++)
        events.pushes.push_back({output_index_base + i, vals[i]});
    
    return index;
}

int layoutMLTREvents(Map& map, int output_index, int input_index_base, const eastl::vector<U64>& vals)
{
    if(output_index >= map.outputs.size())
        return -1;

    U64 sum = 0;

    eastl::vector<Hash> ihashes;
    for(int i = 0; i < vals.size(); i++)
    {
        const Map::Point& input = map.inputs[input_index_base + i];
        ihashes.push_back().calculate(&input.pub.x, sizeof(input.pub.x));
        sum += vals[i];
    }

    Tree xinputs;
    merkleBuild(xinputs, ihashes);

    BuildParamsMULTI xparams {};
    xparams.xchg_class = MLTR_CLASS;
    xparams.addr.dst.encode(map.outputs[output_index].pub);
    xparams.root.ihash = merkleRoot(xinputs);

    int index = map.ids.size();
    ID& xchg_id = map.ids.push_back();
    AEAD& xchg_aead = map.aeads.push_back();
    if(!xchg(&xparams, &xchg_id, &xchg_aead))
        return -1;

    auto& events = map.events[xchg_id];
    for(int i = 0; i < vals.size(); i++)
        events.pulls.push_back({input_index_base + i, vals[i]});

    events.pushes.push_back({output_index, sum});
    return index;
}

int layoutLEDGEREvents(Map& map, int input_index_base, const eastl::vector<U64>& ivals, int output_index_base, const eastl::vector<U64>& ovals)
{
    U64 isum = 0;
    U64 osum = 0;

    eastl::vector<Hash> ihashes;
    for(int i = 0; i < ivals.size(); i++)
    {
        const Map::Point& input = map.inputs[input_index_base + i];
        ihashes.push_back().calculate(&input.pub.x, sizeof(input.pub.x));
        isum += ivals[i];
    }

    eastl::vector<Hash> ohashes;
    for(int i = 0; i < ovals.size(); i++)
    {
        const Map::Point& output = map.outputs[output_index_base + i];
        ohashes.push_back().calculate(&output.pub.x, sizeof(output.pub.x));
        osum += ovals[i];
    }

    Tree xinputs;
    merkleBuild(xinputs, ihashes);

    Tree xoutputs;
    merkleBuild(xoutputs, ohashes);

    BuildParamsLEDGER xparams;
    xparams.xchg_class = LEDGER_CLASS;
    xparams.iroot = merkleRoot(xinputs);
    xparams.oroot = merkleRoot(xoutputs);

    int index = map.ids.size();
    ID& xchg_id = map.ids.push_back();
    AEAD& xchg_aead = map.aeads.push_back();
    if(!xchg(&xparams, &xchg_id, &xchg_aead))
        return -1;

    auto& events = map.events[xchg_id];

    for(int i = 0; i < ivals.size(); i++)
        events.pulls.push_back({input_index_base + i, ivals[i]});

    for(int i = 0; i < ovals.size(); i++)
        events.pushes.push_back({output_index_base + i, ovals[i]});

    return index;
}

rw::SharedBuffer* Build(const Map& map)
{
    rw::AllocatingBuffer data;

    Tree xtree;
    merkleBuild(xtree, map.ids);

    Junction* junc = (Junction*) data.allocateWrite(sizeof(Junction));
    junc->sig_hash = merkleRoot(xtree);
    junc->pin_count = map.inputs.size();
    junc->pout_count = map.outputs.size();
    junc->epull_count = 0;
    junc->epush_count = 0;
    junc->exchange_count = map.aeads.size();

    for(const Map::Point& input : map.inputs)
    {
        Hash ihash;
        ihash.calculate(&input.pub.x, sizeof(input.pub.x));
        data.write(&ihash, sizeof(ihash));
    }

    for(const Map::Point& output : map.inputs)
    {
        Hash ohash;
        ohash.calculate(&output.pub.x, sizeof(output.pub.x));
        data.write(&ohash, sizeof(ohash));
    }
    
    data.write(map.aeads.begin(), map.aeads.size() * sizeof(AEAD));

    int epull_count = 0, epush_count = 0;

    for(const auto& xchg : map.events)
    {
        const ID& id = xchg.first;
        
        const eastl::vector<Event>& ievents = xchg.second.pulls;
        epull_count += ievents.size();

        const eastl::vector<Event>& oevents = xchg.second.pushes;
        epush_count = oevents.size();
        
        data.write((int) ievents.size());
        data.write(ievents.begin(), ievents.size() * sizeof(Event));
        
        data.write((int) oevents.size());
        data.write(oevents.begin(), oevents.size() * sizeof(Event));
    }

    junc->epull_count = epull_count;
    junc->epush_count = epush_count;

    return data.detachSharedBuffer();
    
}
}