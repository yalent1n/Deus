#include <CoreSaturn/DirectoryRecord.h>
#include <cstdio>

namespace Deus
{
char* IANumber::toSimpleString(const IANumber& id, IANumber::SimpleString s)
{
    if(id.zone_id >= c_MAX_ZONES || id.region_id >= c_MAX_REGIONS || id.area_id >= c_MAX_AREAS || id.artifact_id >= c_MAX_ARTIFACTS)
        return NULL;

    snprintf(s, sizeof(IANumber::SimpleString), "%02d%03d%03d%08d", id.zone_id, id.region_id, id.area_id, id.artifact_id);
    return s;
}

char* IANumber::toPrettyString(const IANumber& id, IANumber::PrettyString s)
{
    if(id.zone_id >= c_MAX_ZONES || id.region_id >= c_MAX_REGIONS || id.area_id >= c_MAX_AREAS || id.artifact_id >= c_MAX_ARTIFACTS)
        return NULL;

    snprintf(s, sizeof(IANumber::PrettyString), "%02d%03d%03d-%08d", id.zone_id, id.region_id, id.area_id, id.artifact_id);
    return s;
}

char* IANumber::toDisplayString(const IANumber& id, IANumber::DisplayString s)
{
    if(id.zone_id >= c_MAX_ZONES || id.region_id >= c_MAX_REGIONS || id.area_id >= c_MAX_AREAS || id.artifact_id >= c_MAX_ARTIFACTS)
        return NULL;

    snprintf(s, sizeof(IANumber::DisplayString), "%02d-%03d-%03d-%08d", id.zone_id, id.region_id, id.area_id, id.artifact_id);
    return s;
}
}