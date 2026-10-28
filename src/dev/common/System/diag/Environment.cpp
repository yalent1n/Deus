#include <Engine.Core/diag/Environment.h>
#include <Engine.Base/String/String.h>

#ifdef SvcCompilerMSVC
    #define WIN32_LEAN_AND_MEAN    
    #include <windows.h>
    #include <ShlObj.h>

    #pragma comment(lib, "OneCore.Lib")
#endif

namespace rw
{
void Environment::init()
{

}

eastl::string Environment::getVariable(const char* var)
{
    DWORD valueLength = 0;
    LPSTR value = NULL;
    valueLength = ::GetEnvironmentVariableA(var, value, valueLength);
    if(!valueLength)
    {
        return eastl::string();
    }

    value = (LPSTR) _malloca(valueLength);
    ::GetEnvironmentVariableA(var, value, valueLength);
    return value;
}

bool Environment::setVariable(const char* key, const char* value)
{
    return SetEnvironmentVariableA(key, value);
}

const DbOid& Environment::getSessionOid()
{
    static DbOid sessionOid;
    RUN_ONCE(sessionOid);
    
    sessionOid.generate();
    RAN_ONCE = TRUE;
    
    return sessionOid;
}

char* Environment::getUserId()
{
    DWORD userNameLength = 0;
    LPSTR userName = NULL;
    ::GetUserNameA(userName, &userNameLength);

    userName = (LPSTR) ::malloc(userNameLength);
    ::GetUserNameA(userName, &userNameLength);
    return userName;
}

char* Environment::getHostId()
{
    DWORD hostNameLength = 0;
    LPSTR hostName = NULL;
    ::GetComputerNameExA(ComputerNameDnsHostname, NULL, &hostNameLength);

    hostName = (LPSTR) ::malloc(hostNameLength);
    ::GetComputerNameExA(ComputerNameDnsHostname, hostName, &hostNameLength);
    return hostName;
}

eastl::string Environment::getAppId()
{
    static char appPath[MAX_PATH]{0};
    static DWORD nameIndex = 0;
    RUN_ONCE(appPath + nameIndex);

    HMODULE hApp = GetModuleHandleA(NULL);
    DWORD dwResult = GetModuleFileNameA(hApp, appPath, sizeof appPath);
    EASSERT(dwResult, "Failed to get module file name");
    
    nameIndex = strlen(appPath) - 1;
    while(!issep(appPath[nameIndex]))
        nameIndex--;

    nameIndex++;
    if(size_t underline = indexOf('.', appPath + nameIndex); underline != size_t(-1))
    {
        appPath[nameIndex + underline] = '\0';
    }

    RAN_ONCE = TRUE;
    return appPath + nameIndex;
}

eastl::string Environment::getBranchId()
{
    return getVariable("LOCAL_USER_BRANCH");
}

const char* ConvEnvRel(const char* env, const char* dir)
{
    const char* envVal = Environment::getVariable(env);
    if(isnil(envVal))
        return nullptr;

    StringBuilder<> path;
    path.appendChars(envVal);
    if(!issep(path.c_str()[path.size() - 1]))
        path.appendChar('/');

    path.appendChars(dir);
    fixPath(path.data(), path.size());
    return path.detach();
}

eastl::string Environment::getLocalRootPath()
{
    static const char* localRoot = nullptr;
    RUN_ONCE(localRoot);

    localRoot = ConvEnvRel("LOCAL_ROOT", NULL);
    RAN_ONCE = TRUE;

    return localRoot;
}

eastl::string Environment::getToolRootPath()
{
    static const char* toolsPath = nullptr;
    RUN_ONCE(toolsPath);

    toolsPath = ConvEnvRel("TOOLS_ROOT", NULL);
    RAN_ONCE = TRUE;

    return toolsPath;
}

eastl::string Environment::getDataRootPath()
{
    static const char* dataPath = nullptr;
    RUN_ONCE(dataPath);

    dataPath = ConvEnvRel("DATA_DIR", NULL);
    RAN_ONCE = TRUE;

    return dataPath;
}

eastl::string Environment::getApplicationRootPath()
{
    static const char* appPath = nullptr;
    RUN_ONCE(appPath);

    PWSTR wstr;
    HRESULT hResult = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &wstr);
    EASSERT(SUCCEEDED(hResult), "Failed to get local app data");

    StringBuilder<> appPathBuilder;
    for(size_t i = 0, e = wstrlen(wstr); i < e; ++i)
    {
        char c = (char) wstr[i];
        appPathBuilder.appendChar(c);
    }

    appPathBuilder.appendChars("\\Deus Group\\")
                .appendChars(getAppId().c_str())
                .appendChar('\\');
                
    appPath = appPathBuilder.detach();
    RAN_ONCE = TRUE;
    return appPath;
}
}