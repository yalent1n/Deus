#pragma once
#include <Engine.Base/Platform.h>
#include <Engine.Base/Config.h>

namespace rw
{
class LicenseeInfo
{
public:
    struct Exec {
        const char* project;
        const char* module;
    };
    static inline const Exec DEFAULT_EXEC_INFO = { SvcAppPROJECT, SvcAppMODULE };

    struct Prod {
        PlatformEnum platform;
        BuildConfig config;
    };
    static inline const Prod DEFAULT_PROD_INFO = { CurrentPlatform, BuildConfig_Release };

    struct Internal {
        const char* branch;
        uint changelist;
    };

    virtual const Exec& getTargetInfo() = 0;
    virtual const Prod& getProductInfo() = 0;
    virtual const Internal& getInternalInfo() = 0;    
};
}