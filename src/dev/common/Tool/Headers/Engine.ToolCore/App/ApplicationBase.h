#pragma once
#include "ServiceBase.h"
#include <functional>

#define APPLOG(x) ELOGF("%s: " x, rw::Application::c_app->getDisplayString())
#define APPLOGF(x, ...) ELOGF("%s: " x, rw::Application::c_app->getDisplayString(), __VA_ARGS__)

#define APPERRLOG(x) EERRORF("%s: " x, rw::Application::c_app->getDisplayString())
#define APPERRLOGF(x, ...) EERRORF("%s: " x, rw::Application::c_app->getDisplayString(), __VA_ARGS__)

namespace rw
{
class Application
{
public:
    // Pointer to the current application data.
    static CORETOOL_API Application* c_app;

    CORETOOL_API                      Application();
    CORETOOL_API             virtual ~Application();

    // Returns the application instance as the current application.
    template<typename AppT>
    static inline AppT& getInstance() { return *(AppT*)c_app; }

    // Initialize every manager, allocator, facility, etc. to make the runtime work.
    nodefine bool               InitCore() = 0;

    // Deinitialize it.
    nodefine bool               ShutdownCore() = 0;

    // Returns the branch name this application is written in.
    CORETOOL_API const char*    envLOCAL_USER_PLATFORM() const;

    // Returns the branch name this application is written in.
    CORETOOL_API const char*    envLOCAL_USER_BRANCH() const;

    // Returns the branch name this application is written in.
    CORETOOL_API const char*    envLOCAL_USER_CONFIG() const;

    // Returns the display name for the application. (example: Wind)
    nodefine const char*        getDisplayString() const = 0;

    // Returns the project name for the appliaction. (example: Tool)
    nodefine const char*        getBundleID() const = 0;

    // Iterates through the currently active services.
    CORETOOL_API void        forEachService(std::function<void(const ServiceBase*)> cb);

    // Runs as the main function after initializing the application.
    nodefine int onStart() { return 0; }

    // (optional) Runs as the main update function if the application is a batch app.
    nodefine int onUpdate() { return 0; }

private:
    Vec<ServiceBase*> m_services;
};
}