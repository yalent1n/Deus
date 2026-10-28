#include <Engine.ToolCore/App/ApplicationBase.h>

namespace rw
{
Application* Application::c_app;

Application::Application()
{

}

Application::~Application()
{

}

const char* Application::envLOCAL_USER_PLATFORM() const
{
    return getenv("LOCAL_USER_PLATFORM");
}

const char* Application::envLOCAL_USER_BRANCH() const
{
    return getenv("LOCAL_USER_BRANCH");
}

const char* Application::envLOCAL_USER_CONFIG() const
{
    return getenv("LOCAL_USER_CONFIG");
}

void Application::forEachService(std::function<void(const ServiceBase*)> cb)
{
    for(const ServiceBase* service : m_services)
        cb(service);    
}
}