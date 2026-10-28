#pragma once
#include "frame.h"

namespace rw
{

struct Aidl_Handoff_State {
    ether_addr master_addr;
    ether_addr sync_addr;
    u32 master_metric;
    u32 self_metric;
};

}