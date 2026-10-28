#pragma once
#include "IRenderSurface.h"

namespace rw
{
interface IRenderContext
{
    nodefine void preRenderFrame() = 0;
    nodefine void renderFrame() = 0;
    nodefine void postRenderFrame() = 0;

    nodefine void getViewport(u32* x, u32* y, u32* w, u32* h) const = 0;
    nodefine void setViewport(u32 x, u32 y, u32 w, u32 h) = 0;

    nodefine void clearColor(float r, float g, float b, float a) = 0;
    nodefine void clearDepth(float d) = 0;
    nodefine void clearStencil(u32 v) = 0;
};
}