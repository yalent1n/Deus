#include <Engine.Base/Config.h>
#include <Engine.Base/String/Util.h>

namespace rw
{
BuildConfig toBuildConfig(const char* buildConfig)
{
    if(equalsIgnoreCase("dev", buildConfig))
        return BuildConfig_Dev;
    else if(equalsIgnoreCase("debug", buildConfig))
        return BuildConfig_Debug;
    else if(equalsIgnoreCase("test", buildConfig))
        return BuildConfig_Test;
    else if(equalsIgnoreCase("release", buildConfig))
        return BuildConfig_Release;
    else if(equalsIgnoreCase("shipping", buildConfig))
        return BuildConfig_Shipping;

    return BuildConfig_Invalid;
}

BuildType toBuildType(const char* buildType)
{
    if(equalsIgnoreCase("executable", buildType))
        return BuildType_Executable;
    else if(equalsIgnoreCase("staticlib", buildType))
        return BuildType_StaticLib;
    else if(equalsIgnoreCase("shareddll", buildType))
        return BuildType_SharedDll;

    return BuildType_Invalid;
}
}