#pragma once
#include "Module.h"

namespace Deus
{
inline constexpr s8 c_MAX_ZONES = 100;
inline constexpr s16 c_MAX_REGIONS = 1000;
inline constexpr s16 c_MAX_AREAS = 1000;
inline constexpr s32 c_MAX_ARTIFACTS = 100'000'000;

// DT: Directory Telemetry
// 
// AA-BBB-CCC-DDDDDDDD
// AA: Zone (grand)
// BBB: Region (region)
// CCC: Area (supervisor)
// DDDDDDDD: Record (local)

struct IANumber
{
    typedef char SimpleString[16 + 1];
    typedef char PrettyString[16 + 1 + 1];
    typedef char DisplayString[16 + 3 + 1];

    s32     artifact_id; // level 3
    s16     area_id; // level 2
    s16     region_id; // level 1
    s8      zone_id; // level 0

    static SATURN_API char* toSimpleString(const IANumber&, SimpleString);
    static SATURN_API char* toPrettyString(const IANumber&, PrettyString);
    static SATURN_API char* toDisplayString(const IANumber&, DisplayString);
};

}