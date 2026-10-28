#pragma once
#include "frame.h"

namespace rw
{

struct Aidl_Sync_State {
    clock_t last_update; // microseconds
    u16 sync_offset;
    u16 aw_counter;
    u32 meas_total; // how many times synced
    u16 aw_period;
    u8 presence_mode; // how many EAWs does
};

AIDL_API clock_t mesh80211_next_aw_usec(clock_t now, Aidl_Sync_State* state); 
AIDL_API u16 mesh80211_next_aw_tu(clock_t now, Aidl_Sync_State* state);

AIDL_API int mesh80211_sync_init(Aidl_Sync_State* state);
AIDL_API int mesh80211_sync_update(Aidl_Sync_State* state, clock_t now, u16 time_to_next_aw, u16 aw_counter);

}