#pragma once
#include "../Bundle.h"
#include "../VirtualFileSystem.h"
#include "Environment.h"

#define DEVICE (*rw::ExecutionContext::getVirtualFileSystem())

namespace rw
{
class ExecutionContext
{
public:
    CORE_API static void init(int argc, const char* argv[]);
    CORE_API static void shutdown();

    // ----
    // File system

    // Returns the global file system.
    CORE_API static VirtualFileSystem* getVirtualFileSystem();

    // Botains the backend to the application's root.
    CORE_API static FsBackend* getAppFileSystem();

    // Obtains the backend to the app's local root.
    CORE_API static FsBackend* getAppLocalFileSystem();
    
    // Obtains the backend to the local root.
    // If the workspace is missing return the app's local root directory.
    CORE_API static FsBackend* getLocalFileSystem();
    
    // Obtains the backend to the data root.
    // If the workspace is missing return the app's local Data directory.
    CORE_API static FsBackend* getDataFileSystem();

    // ----
    // Options

    // Denoted with ONE hyphen.
    CORE_API static const char* getOptionValue(const char* option);
    
    // Denoted with ONE or TWO hyphen(s).
    CORE_API static bool getOption(const char* option);
};
}