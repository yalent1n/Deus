#include <Engine.Network/aidl/atlas.h>
#include <Engine.Network/aidl/error.h>

#include <Engine.Base/Buffer/Adapter.h>
#include <Engine.Base/String/String.h>
#include <Engine.Core/diag/ExecutionContext.h>
#include <Engine.Network/Util.h>

namespace rw
{
struct aidl_core
{
    RelayAgent  agent;
    bool        authority;
};

union msf_mtu_buf
{
    u8 bytes[AIDL_FRAME_MTU];
};

__forceinline void _Aidl_sync_init_state(Aidl_Sync_State* sync)
{
    
}

__forceinline void sync_update_tag(aidl_sync_tag* sync)
{
    sync->tx_time = sched_clock_tu();
    sync->aw_counter = mesh80211_get_aw_counter(0);
    u16 mod = (sync->tx_time % sync->aw_period);
    sync->time_to_next_aw = sync->aw_period - __maybe(mod, sync->aw_period - mod);
}

__forceinline void sync_init_tag(aidl_sync_tag* sync, u16 aw_period, u16 eaw_period)
{
    sync->type = AIDL_SYNC_PARAMS_TLV;
    sync->length = sizeof(aidl_sync_tag);

    sync->aw_period = aw_period;
    sync->eaw_period = eaw_period;
    sync_update_tag(sync);
}

__forceinline void discovery_init_tag(aidl_service_discovery_tag* disc, u16 opcode, const char* service, const ether_addr* serviceaddr)
{
    disc->type = AIDL_SERVICE_DISCOVERY_TLV;
    disc->length = sizeof(aidl_service_discovery_tag);
    disc->opcode = opcode;

    if(!isnil(service) && !serviceaddr)
    {
        memcpy(disc->service, service, strlen(service) + 1);
    }
    else
    {
        if(serviceaddr)
        {
            size_t addrlen = (serviceaddr->in.sa_family == AF_INET ? sizeof(serviceaddr->in4) : sizeof(serviceaddr->in6));
            disc->length = sizeof(tl) + sizeof(u8) + addrlen;
            memcpy(&disc->serviceaddr, serviceaddr, addrlen);
        }
        else
        {
            disc->length -= MAX_HOSTNAME_LEN;
        }
    }
}

__forceinline int _Aidl_core_init(Aidl* ai, aidl_core** result, const char* host, const u16 local_port, const u32 mask)
{
    aidl_core* data = (aidl_core*) HeapAllocGlobal(sizeof(aidl_core));
    *result = data;

    // no host, therefore we are origin
    if(isnil(host))
    {
        data->authority = YES;
    }
    else
    {
        data->authority = NO;

        SocketAddress hostaddr;
        resolveHostWithPort(host, hostaddr);

        memcpy(&ai->hostaddr, hostaddr.nativeData, hostaddr.length);
    }

    new (&data->agent) RelayAgent();
    timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt(data->agent.Socket(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    if(!data->agent.Open(local_port, mask))
    {
        return DL_ERR_CLOSED;
    }

    return DL_OK;
}

int _Aidl_init(Aidl* ai, const char* host, const u16 local_port, const u32 mask)
{
    if(!ai)
        return DL_ERR_PARAMETER_NULL;

    _Aidl_sync_init_state(&ai->sync);

    if(int ret = _Aidl_expansion_init_state(&ai->expansion); ret <= DL_ERR_FAILED)
        return ret;

    if(int ret = _Aidl_core_init(ai, &ai->data, host, local_port, mask); ret <= DL_ERR_FAILED)
        return ret;

    return DL_OK;
}

int AidlOpen(Aidl* ai, const u16 local_port, const u32 mask)
{
    return _Aidl_init(ai, NO, local_port, mask);
}

int AidlJoin(Aidl* ai, const char* host, const u16 local_port, const u32 mask)
{
    int ret = _Aidl_init(ai, host, local_port, mask);
    if(ret != DL_OK)
    {
        return ret;
    }

    aidl_expansion_tag ring_tag;
    _Aidl_ring_doorbell(&ai->expansion, &ring_tag);

    ret = AidlSendFrame(ai, { &ring_tag });
    if(ret <= DL_ERR_FAILED)
    {
        return ret;
    }

    for(;;)
    {
        ret = AidlDispatchFrame(ai, NO);
        if(ret == DL_ERR_WANT_READ)
        {
            continue;
        }

        if(ret <= DL_ERR_FAILED)
        {
            return ret;
        }

        break;
    }

    return DL_OK;
}

int AidlWrite(Aidl* ai, const void* bytes, const int length)
{
    if(!ai)
        return DL_ERR_PARAMETER_NULL;

    if(!bytes || !length)
        return DL_ERR_PARAMETER_NULL;

    RelayAgent* agent = &ai->data->agent;
    if(!agent->Write((ai->data->authority ? &ai->dst : &ai->hostaddr), bytes, length))
        return DL_ERR_WANT_WRITE;
    
    return length;
}

int AidlSendFrame(Aidl* ai, const aidl_tags_t& tags)
{
    if(!ai)
    {
        return DL_ERR_PARAMETER_NULL;
    }

    aidl_frame* tx_frame = aidl_frame_new(tags);

    int ret = AidlWrite(ai, tx_frame, sizeof(aidl_frame) + tx_frame->length);
    if(ret <= DL_ERR_FAILED)
    {
        return ret;
    }

    aidl_frame_free(tx_frame);
    return DL_OK;
}

typedef int(*Aidl_TLV_Handler)(Aidl* ai, ether_addr addr, const tl* tag);

static int Aidl_Dispatch_Sync               (Aidl* ai, ether_addr addr, const tl* tag);
static int Aidl_Dispatch_Handoff            (Aidl* ai, ether_addr addr, const tl* tag);
static int Aidl_Dispatch_Key_Exchange       (Aidl* ai, ether_addr addr, const tl* tag);
static int Aidl_Dispatch_Node_Expansion     (Aidl* ai, ether_addr addr, const tl* tag);
static int Aidl_Dispatch_Service_Discovery  (Aidl* ai, ether_addr addr, const tl* tag);
static int Aidl_Dispatch_Artifact_Request   (Aidl* ai, ether_addr addr, const tl* tag);
static int Aidl_Dispatch_Artifact_Response  (Aidl* ai, ether_addr addr, const tl* tag);
static int Aidl_Dispatch_Data               (Aidl* ai, ether_addr addr, const tl* tag);

static Aidl_TLV_Handler s_tlv_dispatch_table[AIDL_TAG_COUNT] {
    Aidl_Dispatch_Sync, // AIDL_SYNC_PARAMS_TLV
    Aidl_Dispatch_Handoff, // AIDL_ELECTION_HANDOFF_TLV
    Aidl_Dispatch_Key_Exchange, // AIDL_KEY_EXCHANGE_TLV
    Aidl_Dispatch_Node_Expansion, // AIDL_NODE_EXPANSION_TLV
    Aidl_Dispatch_Service_Discovery, // AIDL_SERVICE_DISCOVERY_TLV
    Aidl_Dispatch_Artifact_Request, // AIDL_ARTIFACT_REQUEST_TLV
    Aidl_Dispatch_Artifact_Response, // AIDL_ARTIFACT_RESPONSE_TLV
    Aidl_Dispatch_Data, // AIDL_DATA_TLV
};

int _Aidl_handle_tag(Aidl* ai, const ether_addr& addr, const tl* tag, aidl_if* itf)
{
    if(!ai)
    {
        return DL_ERR_PARAMETER_NULL;
    }

    if(!tag)
    {
        return DL_ERR_PARAMETER_NULL;
    }

    Aidl_TLV_Handler handler = s_tlv_dispatch_table[tag->type];
    return handler(ai, addr, tag);
}

int AidlDispatchFrame(Aidl* ai, aidl_if* itf)
{
    if(!ai)
        return DL_ERR_PARAMETER_NULL;

    msf_mtu_buf buf {};

    RelayAgent* agent = &ai->data->agent;
    int ret = (int)agent->Read(&buf, sizeof(msf_mtu_buf), &ai->dst);
    if(!ret)
        return DL_ERR_WANT_READ;

    aidl_frame* frame = (aidl_frame*)buf.bytes;
    if(frame->magic != AIDL_FRAME_MAGIC)
    {
        return DL_ERR_FRAME_MALFORMED;
    }

    aidl_expansion_set(&ai->expansion, ai->dst);
    // aidl_neighbour_table::iterator peer_iter = ai->peers_map.find_by_hash(ether_atoh(&ai->dst));
    // aidl_neighbour* peer = nullptr;
    // if(peer_iter == ai->peers_map.end())
    // {
    //     peer = &ai->peers_map.insert(ether_atoh(&ai->dst)).second;
    // }
    // else
    // {
    //     peer = &peer_iter->second;
    // }

    // peer->addr = ai->dst;

    tl* tag = (tl*)(buf.bytes + sizeof(aidl_frame));
    for(u16 i = 0; i < frame->tag_count; i++)
    { 
        int ret = _Aidl_handle_tag(ai, ai->dst, tag, itf);
        if(ret <= DL_ERR_FAILED)
        {
            return ret;
        }

        tag = (tl*)(((u8*)tag) + tag->length);
    }
    
    return DL_OK;
}

int Aidl_Dispatch_Sync (Aidl* ai, ether_addr addr, const tl* tag)
{
    const aidl_sync_tag* sync = (const aidl_sync_tag*) tag;
    ai->sync.sync_offset = (u16)(sync->tx_time - sched_clock_tu());
    mesh80211_sync_update(&ai->sync, sched_clock_usec(), sync->time_to_next_aw, sync->aw_counter);

    EDEBUGF("mesh: sync %u (tx_time=%zuTU, aw_counter=%u, aw_period=%u, eaw_period=%u, time_to_next_aw=%u)", ai->sync.meas_total, sync->tx_time, ai->sync.aw_counter, ai->sync.aw_period, sync->eaw_period, sync->time_to_next_aw);
    ai->sync.meas_total++;
    return DL_OK;
}

int Aidl_Dispatch_Handoff (Aidl* ai, ether_addr addr, const tl* tag)
{
    return DL_OK;
}

int Aidl_Dispatch_Key_Exchange (Aidl* ai, ether_addr addr, const tl* tag)
{
    return DL_OK;
}

int Aidl_Dispatch_Node_Expansion (Aidl* ai, ether_addr addr, const tl* tag)
{
    const aidl_expansion_tag* expansion = (const aidl_expansion_tag*) tag;

    if(expansion->op == AIDL_EXP_RING_DOORBELL)
    {
        size_t hostname_len = strlen(expansion->host);
        char* hostname = new char[hostname_len + 1];
        memcpy(hostname, expansion->host, hostname_len);
        hostname[hostname_len] = 0;

        EDEBUGF("[aidl] %s has been eager to enter the ExchangeKit.", hostname);
        aidl_expansion_set(&ai->expansion, addr, hostname, &expansion->pub);

        aidl_expansion_tag reply_tag;
        _Aidl_open_door(&ai->expansion, &reply_tag);

        return AidlSendFrame(ai, { &reply_tag });
    }

    if(expansion->op == AIDL_EXP_DOOR_OPENED)
    {
        size_t hostname_len = strlen(expansion->host);
        char* hostname = new char[hostname_len + 1];
        memcpy(hostname, expansion->host, hostname_len);
        hostname[hostname_len] = 0;

        EDEBUGF("[aidl] %s can now enter the ExchangeKit.", hostname);
        ai->expansion.flags |= (ai->expansion.flags & Aidl_Expansion_Estabilished) ? Aidl_Expansion_Estabilished_Multiple : Aidl_Expansion_Estabilished;
    }

    return DL_OK;
}

int Aidl_Dispatch_Service_Discovery (Aidl* ai, ether_addr addr, const tl* tag)
{
    return DL_OK;
}

int Aidl_Dispatch_Artifact_Request (Aidl* ai, ether_addr addr, const tl* tag)
{
    return DL_OK;
}

int Aidl_Dispatch_Artifact_Response (Aidl* ai, ether_addr addr, const tl* tag)
{
    return DL_OK;
}

int Aidl_Dispatch_Data (Aidl* ai, ether_addr addr, const tl* tag)
{
    const aidl_data_tag* data = (const aidl_data_tag*) tag;
    ai->payload_dispatcher(&data->payload[0], tag->length - sizeof(aidl_frame));
    return DL_OK;
}

/*
	RELIABLITY_FACTOR = 5

	PEER CONNECTION LEPESEK:
	1) A mesh-en belul egy peer-tol legyen meg egy publikus IP (legyen PAT (Port Address Translation)), 
	ha viszont KOSNANTNET-en beluli, akkor barmelyik megfelel (101-104).
	2) Erre az IP-re pedig ezt a "tranzakciot" bonyolitjuk le:
		-------------------------- INITIALIZATION --------------------------
		[DEVICE] DEVICE INIT (USER@HOST.DOMAIN formatum, pl. admin@hostcomputer.lan)
		↓
		[MESH] DEVICE ONLINE
		
		---------------------- DAEMON COMMUNICATION ------------------------
		DEVICE							↔								MESH
		
		*SYNC* (936000 × RELIABLITY_FACTOR US interval)		→
		*HANDOFF* (312000 × RELIABLITY_FACTOR US interval)	→
		*PAYLOAD* (72000 us interval. de csak akkor TX ha van adat)
		
	SYNC-nek kell:
		a. TX time (hogy legyen sync offset)
		b. AW counter (melyik AW-ben vagyunk)
*/

int _AidlGetSocket(Aidl* ai, int* socket)
{
    if(!ai)
        return DL_ERR_PARAMETER_NULL;
    
    if(!socket)
        return DL_ERR_PARAMETER_NULL;

    *socket = ai->data->agent.Socket();
    return DL_OK;
}

int AidlClose(Aidl* ai)
{
    if(!ai)
        return DL_ERR_PARAMETER_NULL;

    if(!ai->data)
        return DL_OK;

    if(!HeapFreeGlobal(ai->data))
        return DL_ERR_FAILED;

    ai->data = nullptr;
    return DL_OK;
}
}