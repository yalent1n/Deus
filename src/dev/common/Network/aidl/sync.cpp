#include <Engine.Network/aidl/sync.h>
#include <Engine.Network/aidl/atlas.h>
#include <Engine.Network/aidl/error.h>

namespace rw
{
clock_t mesh80211_next_aw_usec(clock_t now, Aidl_Sync_State* state)
{
    clock_t eaw_period = sched_tu_to_usec(state->presence_mode * state->aw_period);
    clock_t since = now - state->last_update;
    return eaw_period - (since % eaw_period);
}

u16 mesh80211_next_aw_tu(clock_t now, Aidl_Sync_State* state)
{
    clock_t eaw_period = state->presence_mode * state->aw_period;
    clock_t since = sched_usec_to_tu(now - state->last_update);
    return eaw_period - (since % eaw_period);
}

int mesh80211_sync_init(Aidl_Sync_State* state)
{
    state->last_update = sched_clock_usec();
    state->sync_offset = NO;
    state->aw_counter = mesh80211_get_aw_counter(state->sync_offset);
    state->meas_total = 0;
    state->aw_period = AW_PERIOD;
    state->presence_mode = EXTENDED_AW_PERIOD;
}
    
int mesh80211_sync_update(Aidl_Sync_State* state, clock_t now, u16 time_to_next_aw, u16 aw_counter)
{
    if(!state)
    {
        return DL_ERR_PARAMETER_NULL;
    }

    u64 eaw_period = state->presence_mode * state->aw_period;
    state->last_update = now - sched_tu_to_usec(eaw_period - time_to_next_aw);
    state->aw_counter = aw_counter;
    return DL_OK;
}
}