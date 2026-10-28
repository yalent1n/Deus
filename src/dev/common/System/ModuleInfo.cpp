#include <Engine.Core/ModuleInfo.h>
#include <Engine.Base/String/Util.h>

namespace rw
{
ModuleInfo::ModuleInfo(const char* id, const char* name, const char* path)
    : m_name(id)
    , m_friendlyName(name)
    , m_tag(path)
    , m_prevModule(nullptr)
    , m_nextModule(nullptr)
{
    onAttach = nullptr;
    onDetach = nullptr;
    sendMessage = nullptr;
    receiveMessage = nullptr;
}

const char* ModuleInfo::getName() const
{
    return m_name;
}

const char* ModuleInfo::getFriendlyName() const
{
    return m_friendlyName;
}

const char* ModuleInfo::getTag() const
{
    return m_tag;
}

ModuleInfoBinder::ModuleInfoBinder(ModuleInfo* module)
    : m_module(module)
{
    EASSERTF(module->m_nextModule == nullptr, "%s module is already linked", module->getName());
    EASSERTF(module->m_prevModule == nullptr, "%s module is already linked", module->getName());

    if (auto first = ModuleInfo::s_firstModule)
    {
        first->m_prevModule = module;
        module->m_nextModule = first;
    }
    ModuleInfo::s_firstModule = module;
}

ModuleInfoBinder::~ModuleInfoBinder()
{
    ModuleInfo* next = const_cast<ModuleInfo*>(m_module->m_nextModule);
    if (next)
        next->m_prevModule = m_module->m_prevModule;

    if (m_module->m_prevModule)
        const_cast<ModuleInfo*>(m_module->m_prevModule)->m_nextModule = next;
    else
        ModuleInfo::s_firstModule = next;
}

ModuleInfo* ModuleInfo::s_firstModule = nullptr;

void ModuleManager::attach()
{
    ModuleInfo* moduleInfo = ModuleInfo::s_firstModule;
    while(moduleInfo)
    {
        //EDEBUGF("Attached module %s", moduleInfo->getName());
        if(moduleInfo->onAttach)
        {
            moduleInfo->onAttach();
        }

        moduleInfo = moduleInfo->m_nextModule;
    }
}

void ModuleManager::detach()
{
    ModuleInfo* moduleInfo = ModuleInfo::s_firstModule;
    while(moduleInfo)
    {
        //EDEBUGF("Detached module %s", moduleInfo->getName());
        if(moduleInfo->onAttach)
        {
            moduleInfo->onDetach();
        }

        moduleInfo = moduleInfo->m_nextModule;
    }
}

const ModuleInfo* ModuleManager::findModule(const char* name)
{
    ModuleInfo* moduleInfo = ModuleInfo::s_firstModule;
    while(moduleInfo)
    {
        if(equalsIgnoreCase(name, moduleInfo->getName()))
            return moduleInfo;

        moduleInfo = moduleInfo->m_nextModule;
    }

    return nullptr;
}

void ModuleManager::getModules(Vec<const ModuleInfo*>& result)
{
    ModuleInfo* moduleInfo = ModuleInfo::s_firstModule;
    while(moduleInfo)
    {
        result.push_back(moduleInfo);
        moduleInfo = moduleInfo->m_nextModule;
    }
}
}