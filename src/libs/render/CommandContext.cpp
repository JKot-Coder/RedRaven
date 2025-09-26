#include "CommandContext.hpp"

#include "gapi/RenderPassDesc.hpp"

#include "gapi/Buffer.hpp"

#include "gapi/commands/Draw.hpp"
#include "gapi/commands/SetRenderPass.hpp"

#include "render/Effect.hpp"

namespace RR::Render
{

    GAPI::Commands::GeometryLayout& GraphicsCommandContext::GeometryManager::flush(GAPI::CommandList2& commandList)
    {
        if (!dirty)
        {
            if (!currentLayout)
                currentLayout = commandList.allocate<GAPI::Commands::GeometryLayout>();

            return *currentLayout;
        }

        auto vbArray = commandList.allocateArray<GAPI::Commands::VertexBinding>(vertexBindings.size());

        for (size_t i = 0; i < vertexBindings.size(); ++i)
            vbArray[i] = vertexBindings[i];

        currentLayout = commandList.allocate<GAPI::Commands::GeometryLayout>();
        currentLayout->vertexBindings = eastl::span(vbArray, vertexBindings.size());
        currentLayout->indexBuffer = indexBuffer;

        dirty = false;

        return *currentLayout;
    }

    void GraphicsCommandContext::GeometryManager::SetVertexBuffer(uint32_t slot, const GAPI::Buffer& buffer, uint32_t offset = 0)
    {
        if (vertexBindings.size() > slot && vertexBindings[slot].vertexBuffer == &buffer && vertexBindings[slot].vertexBufferOffset == offset)
            return;

        if (vertexBindings.size() <= slot)
            vertexBindings.resize(slot + 1, { nullptr, 0 });

        vertexBindings[slot] = { &buffer, offset };
        dirty = true;
    }

    void GraphicsCommandContext::GeometryManager::SetIndexBuffer(const GAPI::Buffer* buffer)
    {
        if (currentLayout && currentLayout->indexBuffer == buffer)
            return;

        indexBuffer = buffer;
        dirty = true;
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
        GetCommandList().emplaceCommand<GAPI::Commands::Draw>(drawAttribs, pso, flushLayout());
    }

    void GraphicsCommandContext::DrawIndexed(Effect* effect, GAPI::PrimitiveTopology topology, uint32_t startIndex, uint32_t indexCount, uint32_t instanceCount)
    {
        ASSERT(effect);

        auto& layout = flushLayout();
        ASSERT(layout.indexBuffer);

        ASSERT(reinterpret_cast <size_t>(layout.indexBuffer) > 10000) ;

        graphicsParams.primitiveTopology = topology;

        GAPI::Commands::DrawAttribs drawAttribs;
        drawAttribs.vertexCount = indexCount;
        drawAttribs.startLocation = startIndex;
        drawAttribs.instanceCount = instanceCount;

        auto pso = effect->EvaluateGraphicsPipelineState(graphicsParams);
        GetCommandList().emplaceCommand<GAPI::Commands::DrawIndexed>(drawAttribs, pso, layout);
    }
}