#include "CommandContext.hpp"

#include "gapi/RenderPassDesc.hpp"

#include "gapi/Buffer.hpp"

#include "gapi/commands/Draw.hpp"
#include "gapi/commands/SetRenderPass.hpp"

#include "render/Effect.hpp"

namespace RR::Render
{

    void GraphicsCommandContext::SetVertexBuffers(uint32_t slot, const GAPI::Buffer& buffer, uint32_t offset)
    {
        UNUSED(slot, buffer, offset);
        //GetCommandList().emplaceCommand<GAPI::Commands::SetVertexBuffers>(slot, buffer, offset);
    }

    void GraphicsCommandContext::SetIndexBuffer(const GAPI::Buffer* buffer)
    {
        ASSERT(buffer == nullptr || buffer->GetDesc().GetBufferMode() == GAPI::BufferMode::Formatted);
        indexBuffer = buffer;
    }

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

        GAPI::Commands::DrawAttribs drawAttribs;
        drawAttribs.vertexCount = vertexCount;
        drawAttribs.startLocation = startVertex;
        drawAttribs.instanceCount = instanceCount;

        auto pso = effect->EvaluateGraphicsPipelineState(graphicsParams);
        GetCommandList().emplaceCommand<GAPI::Commands::Draw>(drawAttribs, pso);
    }

    void GraphicsCommandContext::DrawIndexed(Effect* effect, GAPI::PrimitiveTopology topology, uint32_t startIndex, uint32_t indexCount, uint32_t instanceCount)
    {
        ASSERT(effect);
        ASSERT(indexBuffer);

        graphicsParams.primitiveTopology = topology;

        GAPI::Commands::DrawAttribs drawAttribs;
        drawAttribs.vertexCount = indexCount;
        drawAttribs.startLocation = startIndex;
        drawAttribs.instanceCount = instanceCount;

        auto pso = effect->EvaluateGraphicsPipelineState(graphicsParams);
        GetCommandList().emplaceCommand<GAPI::Commands::DrawIndexed>(drawAttribs, pso, *indexBuffer);
    }
}