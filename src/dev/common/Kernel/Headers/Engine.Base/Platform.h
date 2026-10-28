#pragma once
#include "Types.h"

namespace rw
{
enum PlatformEnum
{
    // desktop
    Win32,
    Linux,
    Osx,

    // mobile
    Android,
    iOS,

    // america console
    Midway,     // 2001
    Xenon,      // 360
    Durango,    // one
    Anaconda,   // series x

    // japan console
    Executive, // ps1
    EmotionEngine, // ps2
    Cell,       // ps3
    Orbis,      // ps4
    Oberon,     // ps5
    Nx,         // nintendo switch

    // internal mappings
    Gen2a = Midway,     Gen2b = EmotionEngine,
    Gen3a = Xenon,      Gen3b = Cell,
    Gen4a = Durango,    Gen4b = Orbis,
    Gen5a = Anaconda,   Gen5b = Oberon,

    // Note: Currently there are no macros for platform info, so be careful to change it
    // when building any program (or request the platform from CLI)

    #if defined(SvcPlatformWINDOWS)
        CurrentPlatform = Win32
    #elif defined(SvcPlatformLINUX)
        CurrentPlatform = Linux
    #elif defined(SvcPlatformOSX)
        CurrentPlatform = Osx
    #elif defined(SvcPlatformANDROID)
        CurrentPlatform = Android
    #elif defined(SvcPlatformIOS)
        CurrentPlatform = iOS
    #elif defined(SvcPlatformGen2a)
        CurrentPlatform = Gen2a
    #elif defined(SvcPlatformGen2b)
        CurrentPlatform = Gen2b
    #elif defined(SvcPlatformGen3a)
        CurrentPlatform = Gen3a
    #elif defined(SvcPlatformGen3b)
        CurrentPlatform = Gen3b
    #elif defined(SvcPlatformGen4a)
        CurrentPlatform = Gen4a
    #elif defined(SvcPlatformGen4b)
        CurrentPlatform = Gen4b
    #elif defined(SvcPlatformGen5a)
        CurrentPlatform = Gen5a
    #elif defined(SvcPlatformGen5b)
        CurrentPlatform = Gen5b
    #else
        CurrentPlatform = Win32
    #endif
};

constexpr const char* getPlatformName(PlatformEnum platform)
{
    switch(platform)
    {
#if defined(SvcArchitectureX86_64)
        case Win32: return "Win64";
#else
        case Win32: return "Win32";
#endif
        case Linux: return "Linux";
        case Osx: return "Osx";
        case Android: return "Android";
        case iOS: return "iOS";
        case Executive: return "Executive";
        case Nx: return "Nx";
        case Gen2a: return "Midway";
        case Gen2b: return "EE";
        case Gen3a: return "Xenon";
        case Gen3b: return "Cell";
        case Gen4a: return "Durango";
        case Gen4b: return "Orbis";
        case Gen5a: return "Anaconda";
        case Gen5b: return "Oberon";
#if defined(SvcArchitectureX86_64)
        default: return "Win64";
#else
        default: return "Win32";
#endif
    }
}

constexpr const char* getPlatformNameLowercase(PlatformEnum platform)
{
    switch(platform)
    {
#if defined(SvcArchitectureX86_64)
        case Win32: return "win64";
#else
        case Win32: return "win32";
#endif
        case Linux: return "linux";
        case Osx: return "osx";
        case Android: return "android";
        case iOS: return "ios";
        case Executive: return "executive";
        case Nx: return "nx";
        case Gen2a: return "midway";
        case Gen2b: return "ee";
        case Gen3a: return "xenon";
        case Gen3b: return "cell";
        case Gen4a: return "durango";
        case Gen4b: return "orbis";
        case Gen5a: return "anaconda";
        case Gen5b: return "oberon";
#if defined(SvcArchitectureX86_64)
        default: return "win64";
#else
        default: return "win32";
#endif
    }
}
}
