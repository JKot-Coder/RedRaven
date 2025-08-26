#include "CommandContext.hpp"

#include "gapi/RenderPassDesc.hpp"

#include "gapi/commands/Draw.hpp"
#include "gapi/commands/SetRenderPass.hpp"


namespace RR::Render
{
    void GraphicsCommandContext::SetPipelineState(GAPI::GraphicPipelineState* pso)
    {
        this->pso = pso;
    }

    void GraphicsCommandContext::SetRenderPass(const GAPI::RenderPassDesc& renderPass)
    {
        GetCommandList().emplaceCommand<GAPI::Commands::SetRenderPass>(renderPass);
    }

    void GraphicsCommandContext::Draw(GAPI::PrimitiveTopology topology, uint32_t startVertex, uint32_t vertexCount, uint32_t instanceCount)
    {
        GAPI::Commands::Draw::Attribs drawAttribs;
        drawAttribs.vertexCount = vertexCount;
        drawAttribs.startVertex = startVertex;
        drawAttribs.instanceCount = instanceCount;

        UNUSED(topology); // This should be used lately for runtime PSO build here.

        GetCommandList().emplaceCommand<GAPI::Commands::Draw>(drawAttribs, pso);
    }
}