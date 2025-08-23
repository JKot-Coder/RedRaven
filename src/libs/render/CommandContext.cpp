#include "CommandContext.hpp"

#include "gapi/commands/Clear.hpp"
#include "gapi/commands/Draw.hpp"

namespace RR::Render
{
    void GraphicsCommandContext::SetPipelineState(GAPI::GraphicPipelineState* pso)
    {
        this->pso = pso;
    }

    void GraphicsCommandContext::ClearRenderTargetView(const GAPI::RenderTargetView* renderTargetView, const Vector4& color)
    {
        GetCommandList().emplaceCommand<GAPI::Commands::ClearRTV>(renderTargetView, color);
    }

    void GraphicsCommandContext::ClearDepthStencilView(const GAPI::DepthStencilView* depthStencilView, float clearValue)
    {
        GetCommandList().emplaceCommand<GAPI::Commands::ClearDSV>(depthStencilView, clearValue);
    }

    void GraphicsCommandContext::SetFramebuffer(GAPI::Framebuffer* framebuffer)
    {
        this->framebuffer = framebuffer;
    }

    void GraphicsCommandContext::Draw(GAPI::PrimitiveTopology topology, uint32_t startVertex, uint32_t vertexCount, uint32_t instanceCount)
    {
        GAPI::Commands::Draw::Attribs drawAttribs;
        drawAttribs.vertexCount = vertexCount;
        drawAttribs.startVertex = startVertex;
        drawAttribs.instanceCount = instanceCount;

        UNUSED(topology); // This should be used lately for runtime PSO build here.

        GetCommandList().emplaceCommand<GAPI::Commands::Draw>(drawAttribs, pso, framebuffer);
    }
}