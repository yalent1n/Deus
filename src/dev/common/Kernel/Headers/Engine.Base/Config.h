#pragma once
#include "Types.h"

namespace rw
{
enum BuildConfig : uint
{
    BuildConfig_Invalid,

    BuildConfig_Debug,
    BuildConfig_Dev,
    BuildConfig_Test,
    BuildConfig_Release,
    BuildConfig_Shipping,

    BuildConfig_Default = BuildConfig_Release,

#if defined(BUILD_CONFIG_DEBUG)
    CurrentBuildConfig = BuildConfig_Debug
#elif defined(BUILD_CONFIG_DEV)
    CurrentBuildConfig = BuildConfig_Dev
#elif defined(BUILD_CONFIG_TEST)
    CurrentBuildConfig = BuildConfig_Test
#elif defined(BUILD_CONFIG_RELEASE)
    CurrentBuildConfig = BuildConfig_Release
#elif defined(BUILD_CONFIG_SHIPPING)
    CurrentBuildConfig = BuildConfig_Shipping
#else
    CurrentBuildConfig = BuildConfig_Default
#endif
};

enum BuildType : uint
{
    BuildType_Invalid,

    BuildType_Executable, // PKG, EXE, XVC
    BuildType_StaticLib, // LIB
    BuildType_SharedDll, // DLL, DYLIB

    BuildType_Default = BuildType_Executable,

#if defined(BUILD_TYPE_EXECUTABLE)
    CurrentBuildType = BuildType_Executable
#elif defined(BUILD_TYPE_STATIC_LIBRARY)
    CurrentBuildType = BuildType_StaticLib
#elif defined(BUILD_TYPE_SHARED_DLL)
    CurrentBuildType = BuildType_SharedDll
#else
    CurrentBuildType = BuildType_Default
#endif
};

LIB_LINKAGE BuildConfig toBuildConfig(const char* buildConfig);

constexpr const char* getConfigName(BuildConfig config)
{
    switch(config)
    {
        case BuildConfig_Debug: return "Debug";
        case BuildConfig_Dev: return "Dev";
        case BuildConfig_Test: return "Test";
        case BuildConfig_Release: return "Release";
        case BuildConfig_Shipping: return "Shipping";
        default: return nullptr;
    }
}

constexpr const char* getConfigNameLowercase(BuildConfig config)
{
    switch(config)
    {
        case BuildConfig_Debug: return "debug";
        case BuildConfig_Dev: return "dev";
        case BuildConfig_Test: return "test";
        case BuildConfig_Release: return "release";
        case BuildConfig_Shipping: return "shipping";
        default: return nullptr;
    }
}

LIB_LINKAGE BuildType toBuildType(const char* buildType);

constexpr const char* getBuildTypeName(BuildType type)
{
    switch(type)
    {
        case BuildType_Executable: return "Executable";
        case BuildType_StaticLib: return "StaticLib";
        case BuildType_SharedDll: return "SharedDll";
        default: return nullptr;
    };
}

constexpr const char* getBuildTypeNameLowercase(BuildType type)
{
    switch(type)
    {
        case BuildType_Executable: return "executable";
        case BuildType_StaticLib: return "staticlib";
        case BuildType_SharedDll: return "shareddll";
        default: return nullptr;
    };
}
}