#include "CommandContext.hpp"

#include "gapi/RenderPassDesc.hpp"

#include "gapi/commands/Draw.hpp"
#include "gapi/commands/SetRenderPass.hpp"

#include "render/Effect.hpp"

namespace RR::Render
{
    void GraphicsCommandContext::SetRenderPass(const GAPI::RenderPassDesc& renderPass)
    {
        GetCommandList().emplaceCommand<GAPI::Commands::SetRenderPass>(renderPass);

        graphicsParams.renderTargetCount = renderPass.colorAttachmentCount;

        ASSERT( graphicsParams.renderTargetFormats.size()  == graphicsParams.renderTargetFormats.size());
        for(size_t i = 0; i < renderPass.colorAttachmentCount; ++i)
        {
            const auto& colorAttachment = renderPass.colorAttachments[i];
            const auto* renderTargetView = colorAttachment.renderTargetView;
            graphicsParams.renderTargetFormats[i] = renderTargetView ? renderTargetView->GetDesc().format : GAPI::GpuResourceFormat::Unknown;
        }

        const auto* depthStencilView = renderPass.depthStencilAttachment.depthStencilView;
        graphicsParams.depthStencilFormat = depthStencilView ? depthStencilView->GetDesc().format : GAPI::GpuResourceFormat::Unknown;
    }

    void GraphicsCommandContext::Draw(Effect* effect, GAPI::PrimitiveTopology topology, uint32_t startVertex, uint32_t vertexCount, uint32_t instanceCount)
    {
        ASSERT(effect);
        graphicsParams.primitiveTopology = topology;

        GAPI::Commands::Draw::Attribs drawAttribs;
        drawAttribs.vertexCount = vertexCount;
        drawAttribs.startVertex = startVertex;
        drawAttribs.instanceCount = instanceCount;

        UNUSED(topology); // This should be used lately for runtime PSO build here.

        auto pso = effect->EvaluateGraphicsPipelineState(graphicsParams);
        GetCommandList().emplaceCommand<GAPI::Commands::Draw>(drawAttribs, pso);
    }
}