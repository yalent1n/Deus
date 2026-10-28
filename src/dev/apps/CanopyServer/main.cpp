#include "App.h"
#include <Engine.Reflection/Compartment.h>

int main(int argc, const char* argv[])
{    
    rw::ACanopyServer app(argc, argv);
    rw::Application::c_app = &app;
    int retCode = app.onStart();
    return retCode;
}