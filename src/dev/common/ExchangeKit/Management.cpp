#include <CoreSaturn/Management.h>

namespace Deus::Management
{
static ekvm_Provider_object* s_obj_prov = nullptr;
static ekvm_Provider_data* s_data_prov = nullptr;

ekvm_Provider& providerOBJECT()
{
    RUN_ONCE(*s_obj_prov);

    s_obj_prov = new ekvm_Provider_object();

    RAN_ONCE = YES;
    return *s_obj_prov;
}

ekvm_Provider& providerDATA()
{
    RUN_ONCE(*s_data_prov);

    s_data_prov = new ekvm_Provider_data();

    RAN_ONCE = YES;
    return *s_data_prov;
}
}