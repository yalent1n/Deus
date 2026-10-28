#pragma once
#include "../Bundle.h"
#include <Engine.Base/Database/DbOid.h>

namespace rw
{
class Environment
{
public:
    CORE_API static void init();

    // ----
    // Environment variables

    // Obtains the variable from the environemnt. 
    // Returns "" if not found.
    CORE_API static eastl::string getVariable(const char*);

    // Sets the variable in the environment.
    CORE_API static bool setVariable(const char*, const char*);

    // ----
    // Identifiers

    // Identifies the application instance in any session-based service.
    CORE_API static const DbOid& getSessionOid();

    // Returns the current user's name.
    CORE_API static char* getUserId();

    // Returns this device's name.
    CORE_API static char* getHostId();

    // Returns the name of the executable.
    CORE_API static eastl::string getAppId();

    // Returns a branch identifier for toolchain libs.
    CORE_API static eastl::string getBranchId();
    
    // ----
    // Directories

    // Root of the 'local' directory in the environment.
    CORE_API static eastl::string getLocalRootPath();

    // Root of the 'tools' directory in the environment.
    CORE_API static eastl::string getToolRootPath();

    // Root of the 'data' directory in the environment.
    CORE_API static eastl::string getDataRootPath();

    // Root of the app directory in the Windows environment.
    CORE_API static eastl::string getApplicationRootPath();
};
}