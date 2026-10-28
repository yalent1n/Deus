#pragma once
#include "../Bundle.h"
#include "../Relay.h"
#include <Engine.Base/Misc/Time.h>
#include "../Netcrypt.h"
#include <Engine.Base/Crypto/Base64.h>

#define MAX_HOSTNAME_LEN 63
#define MAX_SERVICENAME_LEN 31
#define AIDL_API FB_NETWORK_API

// NEW NAME: mesh80211 -> Aidl (Aether Interfaced Data Layer)

//////////////////////////

enum aidl_tlvs
{
    AIDL_SYNC_PARAMS_TLV,
    AIDL_ELECTION_HANDOFF_TLV,
    AIDL_KEY_EXCHANGE_TLV,
    AIDL_NODE_EXPANSION_TLV,
    AIDL_SERVICE_DISCOVERY_TLV,
    AIDL_ARTIFACT_REQUEST_TLV,
    AIDL_ARTIFACT_RESPONSE_TLV,
    AIDL_DATA_TLV,

    AIDL_TAG_COUNT
};

// Frame Properties
#define AIDL_FRAME_MAGIC                    (0x00A0A1DL)
#define AIDL_FRAME_MTU                      (1400u)

// Crypto Properties
#define AIDL_DATA_PROVIDER_LENGTH           (16)
#define AIDL_DATA_KEY_LENGTH                (16)

// Sync Properties
#define _TU                                     (1024)
#define AW_PERIOD                               (16)
#define AW_PERIOD_TU                            (AW_PERIOD * _TU)
#define EXTENDED_AW_PERIOD                      (4)
#define EXTENDED_AW_PERIOD_TU                   (EXTENDED_AW_PERIOD * _TU)

//////////////////////////

namespace rw
{
typedef Timestamp clock_t;

struct aidl_frame {
    u32 magic;
    u16 tag_count;
    u16 length;
};

struct tl {
    u16 type;
    u16 length;
};

enum class peer_action : u8 { connect, disconnect };

// TODO: add some interface with _if at the end
struct aidl_if {
    void(*peer_handler)(void* /*state*/, void* /*peer*/, peer_action);
};

// what does a sync need?
struct aidl_sync_tag : tl {
    clock_t tx_time;
    u32 aw_counter;
    u8 aw_period;
    u8 eaw_period; // length of an extended AW
    u8 time_to_next_aw;
    u8 pad;
    ether_addr sync_master;
};

// elect a new master every PI (like a rotation cycle)
struct aidl_handoff_election_tag : tl {
    ether_addr master_addr; // current master address
    ether_addr sync_addr; // sync-to master address

    u32 id;
    u32 master_metric;
    u32 self_metric;
};

struct aidl_key_exchange_tag : tl {
    Deus::Exchange::Key k;
};

#define AIDL_EXP_RING_DOORBELL 1
#define AIDL_EXP_DOOR_OPENED 2

struct aidl_expansion_tag : tl {
    Deus::Exchange::Address pub;
    u8 op;
    char host[MAX_HOSTNAME_LEN];
};
inline auto x = sizeof(aidl_expansion_tag);
#define AIDL_SD_ASK_WHERE 1
#define AIDL_SD_ANSWER_BACK 2

struct aidl_service_discovery_tag : tl {
    u8 opcode;
    union {
        ether_addr serviceaddr;
        char service[MAX_SERVICENAME_LEN];
    };
};

struct aidl_artifact_request_tag : tl {
    uint16_t key_count;
    SHA1 keys[];
};

enum aidl_artifact_flags : uint16_t {
    mesh80211_data_file = 0x01,
    mesh80211_data_delta = 0x02,
    mesh80211_data_crypto = 0x04
};

struct aidl_artifact_entry {
    uint32_t    file_id; // cas file ID
    uint16_t    crypto_id; // crypto entry index
    uint16_t    flags; // enum above
};

struct aidl_crypto_entry {
    u8 provider_id[AIDL_DATA_PROVIDER_LENGTH]; // S3 bucket ID
    u8 key_id[AIDL_DATA_KEY_LENGTH]; // S3 bucket key
};

struct aidl_artifact_response_tag : tl {
    uint16_t entry_count;
    uint16_t crypto_count;
};

struct aidl_data_tag : tl {
    u8 payload[0];
};

typedef eastl::vector<tl*> aidl_tags_t;

AIDL_API aidl_frame* aidl_frame_new(const aidl_tags_t &tags);
AIDL_API aidl_frame* aidl_frame_free(aidl_frame* frame);
}