#pragma once
#include "IRenderDevice.h"
#include "WidgetRenderSurface.h"

namespace rw::widgets
{
class UIContext : public IRenderContext
{
public:
    FB_RENDER_API UIContext();

    FB_RENDER_API void preRenderFrame() override;
    FB_RENDER_API void renderFrame() override;
    FB_RENDER_API void postRenderFrame() override;

    FB_RENDER_API void getViewport(u32* x, u32* y, u32* w, u32* h) const override;
    FB_RENDER_API void setViewport(u32 x, u32 y, u32 w, u32 h) override;

    FB_RENDER_API void clearColor(float r, float g, float b, float a) override;
    FB_RENDER_API void clearDepth(float d) override;
    FB_RENDER_API void clearStencil(u32 v) override;

public:
    FB_RENDER_API void addSurface(UIPanel *);

private:
    eastl::vector<UIPanel*> m_Collection;
};
}