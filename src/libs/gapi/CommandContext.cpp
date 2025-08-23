#include "CommandContext.hpp"

#include "gapi/commands/Clear.hpp"
#include "gapi/commands/Draw.hpp"

namespace RR::GAPI
{
    void GraphicsCommandContext::SetPipelineState(GraphicPipelineState* pso)
    {
        this->pso = pso;
    }

    void GraphicsCommandContext::ClearRenderTargetView(const RenderTargetView* renderTargetView, const Vector4& color)
    {
        GetCommandList().emplaceCommand<Commands::ClearRTV>(renderTargetView, color);
    }

    void GraphicsCommandContext::ClearDepthStencilView(const DepthStencilView* depthStencilView, float clearValue)
    {
        GetCommandList().emplaceCommand<Commands::ClearDSV>(depthStencilView, clearValue);
    }

    void GraphicsCommandContext::SetFramebuffer(Framebuffer* framebuffer)
    {
        this->framebuffer = framebuffer;
    }

    void GraphicsCommandContext::Draw(PrimitiveTopology topology, uint32_t startVertex, uint32_t vertexCount, uint32_t instanceCount)
    {
        Commands::Draw::Attribs drawAttribs;
        drawAttribs.vertexCount = vertexCount;
        drawAttribs.startVertex = startVertex;
        drawAttribs.instanceCount = instanceCount;

        UNUSED(topology); // This should be used lately for runtime PSO build here.

        GetCommandList().emplaceCommand<Commands::Draw>(drawAttribs, pso, framebuffer);
    }
}