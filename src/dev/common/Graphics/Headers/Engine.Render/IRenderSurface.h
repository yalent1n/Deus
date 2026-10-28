#pragma once
#include "Module.h"

namespace rw
{
interface IRenderSurface
{
    nodefine void getFrameSize(int* x, int* y) const = 0;
    nodefine void drawFrame() = 0;
};
}