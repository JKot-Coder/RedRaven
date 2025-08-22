#include "CommandList2.hpp"

#include "gapi/commands/Clear.hpp"
#include "gapi/commands/Draw.hpp"

namespace RR::GAPI
{
    void GraphicsOperationsMixin::SetPipelineState(GraphicPipelineState* pso)
    {
        this->pso = pso;
    }

    void GraphicsOperationsMixin::ClearRenderTargetView(const RenderTargetView* renderTargetView, const Vector4& color)
    {
        GetCommandList().emplaceCommand<Commands::ClearRTV>(renderTargetView, color);
    }

    void GraphicsOperationsMixin::ClearDepthStencilView(const DepthStencilView* depthStencilView, float clearValue)
    {
        GetCommandList().emplaceCommand<Commands::ClearDSV>(depthStencilView, clearValue);
    }

    void GraphicsOperationsMixin::SetFramebuffer(Framebuffer* framebuffer)
    {
        this->framebuffer = framebuffer;
    }

}