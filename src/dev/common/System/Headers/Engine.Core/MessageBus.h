#pragma once
#include "Bundle.h"
#include <Engine.Base/HashMap.h>

namespace rw
{
typedef word ControlCode;
enum : ControlCode
{
    CcSend,
    CcRecv,
    CcSub,
    CcUnsub,
    
    CcSystemReserved    = 1024,
    CcSuccess           = CcSystemReserved,
    CcError             = CcSystemReserved - 1,
    CcPending           = CcSystemReserved - 2
};

typedef ControlCode(*ControlDispatcher_t)(const void* in, size_t cbin, void* out, size_t cbout);
typedef Dict<ControlCode, ControlDispatcher_t> ControlDispatchers_t;

struct MessageBus
{
    typedef u8 Page_t[4096];

    u8* page;
    ControlDispatchers_t controlTable;
};

CORE_API bool subscribeToBus(MessageBus& bus);
CORE_API bool unsubscribeFromBus(MessageBus& bus);

// ControlCode ret = BusIoControl(BaseBundle.bus, CcSend, msg, sizeof(msg) - 1, buf, sizeof(buf), &bytesSent);
CORE_API ControlCode BusIoControl(
    MessageBus& bus, ControlCode cc,
    const void* in, size_t cbin,
    void* out, size_t cbout,    
    size_t* bytesReturned = NIL
);


}