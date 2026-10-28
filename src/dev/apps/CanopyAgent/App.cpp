#include "App.h"

#include <Engine.Base/Memory/VirtualAlloc.h>
#include <Engine.Base/Memory/HeapAllocator.h>

#include <Engine.Base/Database/DbOid.h>
#include <Engine.Core/diag/ExecutionContext.h>
#include <Engine.Reflection/Compartment.h>
#include <Engine.Reflection/NativeTypeRegistry.h>
#include <Engine.Reflection/GenericDataLayout.h>
#include <Engine.Reflection/RuntimeIpr.h>
#include <Engine.Storage/IoService.h>
#include <Engine.Storage/IoTrustVault.h>

namespace rw
{
static inline ResourceCompartment s_staticCompartment = { Compartment_STATIC, "app.StaticArena", 16 MiB };

ACanopyAgent::ACanopyAgent(int argc, const char* argv[])
    : m_argv(argv)
    , m_argc(argc)
{
    InitCore();
}

ACanopyAgent::~ACanopyAgent()
{
    ShutdownCore();
}

bool ACanopyAgent::InitCore()
{
    m_tlsContext = CSSkywalkStart();

    DbOid::init();

#if ENGINE_USE_ALLOCATOR
    if(!sysInitializeHeapAllocator())
    {
        EERROR("init: initializing global heap allocator failed");
        return false;
    }
#else
    if(!InitGlobalHeap()) {
        EERROR("init: initializing global heap failed");
        return false;
    }
#endif

    ResourceCompartment::linkActive(Compartment_STATIC);
    g_nativeTypeRegistry = new NativeTypeRegistry();

    ExecutionContext::init(m_argc, m_argv);

    // TODO: how to decide if we wan't Runtime or Pipeline IPR
    RuntimeIpr::staticInit();

    // (void) _DgscKeyManager();
    // (void) ioService();
    return true;
}

bool ACanopyAgent::ShutdownCore()
{
    // delete ioService();
    // delete _DgscKeyManager();

    ExecutionContext::shutdown();
    delete g_nativeTypeRegistry;
    ResourceCompartment::unlinkActive();

#ifdef ENGINE_USE_ALLOCATOR
    if(!sysDeInitializeHeapAllocator())
    {
        EERROR("shutdown: failed to shut down the global heap allocator");
        return false;
    }
#else
    if(!ShutdownGlobalHeap()) {
        EERROR("shutdown: failed to shut down the global heap");
        return false;
    }
#endif

    return CSSkywalkStop();
}

const char* ACanopyAgent::getBundleID() const
{
    return "com.konsant.CanopyAgent";
}

const char* ACanopyAgent::getDisplayString() const
{
    return "Canopy Agent";
}

void ACanopyAgent::enableBatch()
{
    m_options.batch = 1;
}

bool ACanopyAgent::batchEnabled()
{
    return !!(m_options.batch);
}
}