#pragma once
#include "../Bundle.h"
#include <Engine.Base/Vec.h>

namespace rw
{
interface ServiceBase
{
    CORETOOL_API               ServiceBase();
    CORETOOL_API     nodefine ~ServiceBase();

    nodefine const char* getFriendlyName() const = 0;
};
}