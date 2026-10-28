#pragma once
#include <Engine.Base/Vec.h>
#include <Engine.Base/Ref.h>

#define FB_DECLARE_MODULE(bid) \
class bid ## _Module { \
public: \
    static FB_ ##bid## _API ::rw::ModuleInfo c_moduleInfo; \
};

#define FB_IMPLEMENT_MODULE(bid, bname) \
::rw::ModuleInfo bid ## _Module::c_moduleInfo(#bid, bname, "<null>"); \
const ::rw::ModuleInfo& c_##bid##_ModuleInfo = bid##_Module ::c_moduleInfo; \
static ::rw::ModuleInfoBinder bid ## Dummy(&bid##_Module::c_moduleInfo);

namespace rw
{
typedef unsigned ErrorNo;

// TODO: resolve the error where the module info won't be compiled
class ModuleInfo 
{
    friend struct ModuleInfoBinder;
    friend class ModuleManager;

public:
    const unsigned kMaxMessageLength = 2048;

    LIB_LINKAGE ModuleInfo();
    LIB_LINKAGE ModuleInfo(const char* name, const char* friendlyName, const char* tag);

    // User functions
    ErrorNo(*onAttach)();
    ErrorNo(*onDetach)();
    ErrorNo(*sendMessage)(ModuleInfo* targetBundle, const void* bytes, unsigned length);
    ErrorNo(*receiveMessage)(void* destBytes, unsigned length);

    // Sealed functions
    LIB_LINKAGE const char* getName() const;        // bundle id (ex.: core)
    LIB_LINKAGE const char* getFriendlyName() const;              // (?) maybe some display name for it (ex.: "Core Package")
    LIB_LINKAGE const char* getTag() const;              // (?) could be the path if linking as DLL (ex.: "ext/Core.bundle")

    static ModuleInfo* s_firstModule;

private:
    const char* m_name;
    const char* m_friendlyName;
    const char* m_tag;

    ModuleInfo* m_prevModule;
    ModuleInfo* m_nextModule;
};

struct ModuleInfoBinder
{
    LIB_LINKAGE ModuleInfoBinder(ModuleInfo* module);
    LIB_LINKAGE ~ModuleInfoBinder();

    ModuleInfo* m_module;
};

#if defined(ENGINE_CORE_EXPORTS)
    #define CORE_API LIB_LINKAGE
#elif defined(ENGINE_BUILD_STATIC)
    #define CORE_API
#else
    #ifdef SvcCompilerMSVC
        #define CORE_API __declspec(dllimport)
    #else
        #define CORE_API __attribute__((visibility("default")))
    #endif
#endif

#define FB_Core_API CORE_API
FB_DECLARE_MODULE(Core)
#undef FB_Core_API

class ModuleManager
{
public:
    CORE_API static void attach();
    CORE_API static void detach();

    CORE_API static const ModuleInfo* findModule(const char* name);
    CORE_API static void getModules(Vec<const ModuleInfo*>& result);
};
}