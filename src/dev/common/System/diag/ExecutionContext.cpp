#include <Engine.Core/diag/ExecutionContext.h>
#include <Engine.Core/VirtualFileSystem.h>
#include <Engine.Base/HashMap.h>
#include <Engine.Base/Log.h>

#ifdef SvcCompilerMSVC
    #define WIN32_LEAN_AND_MEAN    
    #include <windows.h>
#endif

#include <stdlib.h>

namespace rw
{
struct ExecutionContextData
{    
    char*               nativeLocalPath;
    char*               nativeWorkingPath;
    
    VirtualFileSystem*  vfs;
    
    // general backends
    FsBackend*          appRootDir;
    FsBackend*          appLocalDir;
    FsBackend*          workingDir;

    // toolchain backends
    FsBackend*          dataDir;
    FsBackend*          localDir;

    const char**        argv;
    int                 argc;

    Dict<const char*, const char*, HashStringNoCase> options;
};

static ExecutionContextData s_ecData {};

void parseVar(const char* key, const char* value)
{
    VarsVar* var = Vars::getVar(key);
    if(!var)
        return;

    if(u8(var->type) & u8(VarT::Integer))
    {
        bool result = true;

        switch(var->memberSize)
        {
            case 1: 
            {
                if((u8)var->type & (u8)VarT::SignedFlag) {
                    result = tointeger(value, *(s8*)var->rawValue);
                } else {
                    result = tointeger(value, *(u8*)var->rawValue);
                }
                break;
            }
            case 2: 
            {
                if((u8)var->type & (u8)VarT::SignedFlag) {
                    result = tointeger(value, *(s16*)var->rawValue);
                } else {
                    result = tointeger(value, *(u16*)var->rawValue);
                }
                break;
            }
            case 4:
            {
                if((u8)var->type & (u8)VarT::SignedFlag) {
                    result = tointeger(value, *(s32*)var->rawValue);
                } else {
                    result = tointeger(value, *(u32*)var->rawValue);
                }
                break;
            }
            case 8: 
            {
                if((u8)var->type & (u8)VarT::SignedFlag) {
                    result = tointeger(value, *(s64*)var->rawValue);
                } else {
                    result = tointeger(value, *(u64*)var->rawValue);
                }
                break;
            }
            default: 
                break;
        }

        EASSERTF(result, "Failed to convert the value of %s to integer", key);
        return;
    }
    
    if(var->type == VarT::Float)
    {
        double num = atof(value);
        switch(var->memberSize)
        {
            case 4: *(float*)var->rawValue = float(num); break;
            case 8: *(double*)var->rawValue = num; break;
            default: break;
        }

        return;
    }

    if(var->type == VarT::String)
    {
        *(const char**)var->rawValue = value;
        return;
    }

    if(var->type == VarT::Boolean)
    {
        *(bool*)var->rawValue = equalsIgnoreCase("true", value) || equalsIgnoreCase("yes", value);
    }
}

void parseCommandLine(int argc, const char* argv[])
{
    s_ecData.argc = argc;
    s_ecData.argv = argv;
    
    for(int i = 1; i < argc; i++)
    {
        const char* key = argv[i];
        const char* value = nullptr;
        if(key[0] == '-') // option
        {
            ++key;
            if(key[0] != '-')
            {
                value = argv[++i];
                __builtin_assert(i <= argc);
            } else
            {
                ++key;
                value = nullptr;
            }
            
            s_ecData.options.insert(key, value);
        }
        else if(key[0] == '$')
        {
            ++key;
            value = argv[++i];
            __builtin_assert(i <= argc);

            if(!isnil(key))
            {
                Environment::setVariable(key, value);
            }
        }
        else
        {
            value = argv[++i];
            __builtin_assert(i <= argc); // buffer overflow would happen either
            parseVar(key, value);
        }
    }
}

VirtualFileSystem* ExecutionContext::getVirtualFileSystem()
{
    EASSERT(s_ecData.vfs, "Virtual file system is not initialized");
    return s_ecData.vfs;
}

FsBackend* ExecutionContext::getAppFileSystem()
{
    EASSERT(s_ecData.localDir, "App root directory is not initialized");
    return s_ecData.appRootDir;
}

FsBackend* ExecutionContext::getAppLocalFileSystem()
{
    EASSERT(s_ecData.localDir, "App local directory is not initialized");
    return s_ecData.appLocalDir;
}

FsBackend* ExecutionContext::getLocalFileSystem()
{
    EASSERT(s_ecData.localDir, "Local directory is not initialized");
    return s_ecData.localDir;
}

FsBackend* ExecutionContext::getDataFileSystem()
{
    EASSERT(s_ecData.dataDir, "Data directory is not initialized");
    return s_ecData.dataDir;
}

struct StringNoCaseComparer
{
    inline bool operator()(const char* s1, const char* s2) const
    {
        return equalsIgnoreCase(s1, s2);
    }
};

const char* ExecutionContext::getOptionValue(const char* option)
{
    if(isnil(option))
        return nullptr;

    auto opt = s_ecData.options.find(option);
    return (opt != s_ecData.options.end()) ? opt->second : nullptr;
}

bool ExecutionContext::getOption(const char* option)
{
    if(isnil(option))
        return false;

    auto opt = s_ecData.options.find(option);
    return opt != s_ecData.options.end();
}

void ExecutionContext::init(int argc, const char* argv[])
{
    ModuleManager::attach();
    DbOid::init();
    parseCommandLine(argc, argv);

    s_ecData.nativeLocalPath = new char[MAX_PATH];
    s_ecData.nativeWorkingPath = new char[MAX_PATH];

    DWORD Ret = GetModuleFileNameA(NIL, s_ecData.nativeLocalPath, MAX_PATH);
    EASSERT(Ret, "Failed to get module file name");
    s_ecData.nativeLocalPath[Ret] = '\0';

    {
        size_t szlen = strlen(s_ecData.nativeLocalPath);
        char* cursor = s_ecData.nativeLocalPath + szlen - 1;
        while(*cursor != '/' && *cursor != '\\')
            cursor--;

        *cursor++ = '\\';
        *cursor = '\0';
    }
    
    VirtualFileSystem* vfs = new VirtualFileSystem();
    s_ecData.vfs = vfs;

    s_ecData.appRootDir = vfs->mount("app:", Environment::getApplicationRootPath().c_str());
    s_ecData.appLocalDir = vfs->mount("localapp:", s_ecData.nativeLocalPath);
    s_ecData.workingDir = vfs->mount("cwd:", [&] 
    { 
        DWORD Ret = GetCurrentDirectoryA(MAX_PATH, s_ecData.nativeWorkingPath);
        EASSERT(Ret, "Failed to get working directory");
        s_ecData.nativeWorkingPath[Ret++] = '\\';
        s_ecData.nativeWorkingPath[Ret] = '\0';
        return s_ecData.nativeWorkingPath;
    });

    const char* dataDirPath = Environment::getVariable("DATA_ROOT");

    if(isnil(dataDirPath))
    {
        StringBuilder<> localDataDir;
        localDataDir << s_ecData.nativeLocalPath << "Data\\";
        dataDirPath = localDataDir.detach();
    }

    s_ecData.dataDir = vfs->mount("data:", dataDirPath);
}

void ExecutionContext::shutdown()
{
    if(s_ecData.localDir) {
        s_ecData.vfs->unmount(s_ecData.localDir);
        s_ecData.localDir = nullptr;
    }
    if(s_ecData.workingDir) {
        s_ecData.vfs->unmount(s_ecData.workingDir);
        s_ecData.workingDir = nullptr;
    }
    if(s_ecData.dataDir) {
        s_ecData.vfs->unmount(s_ecData.dataDir);
        s_ecData.dataDir = nullptr;
    }

    ModuleManager::detach();
}
}