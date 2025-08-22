#include "CommandList2.hpp"

#include "gapi/commands/Clear.hpp"

namespace RR::GAPI
{
    void GraphicsOperationsMixin::ClearRenderTargetView(const RenderTargetView* renderTargetView, const Vector4& color)
    {
        GetCommandList().emplaceCommand<Commands::ClearRTV>(renderTargetView, color);
    }

    void GraphicsOperationsMixin::SetFramebuffer(Framebuffer* framebuffer)
    {
        this->framebuffer = framebuffer;
    }

}