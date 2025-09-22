#include "CommandListImpl.hpp"

#include "DeviceContext.h"

#include "GpuResourceViewImpl.hpp"
#include "PipelineStateImpl.hpp"
#include "GpuResourceImpl.hpp"

#include "gapi/RenderPassDesc.hpp"

#include "gapi/Buffer.hpp"
#include "gapi/commands/Draw.hpp"
#include "gapi/commands/SetRenderPass.hpp"

namespace DL = ::Diligent;

namespace RR::GAPI::Diligent
{
    namespace
    {
        struct CommandCompileContext
        {
            DL::IDeviceContext* device;
        };

        void compileCommand(const Commands::SetRenderPass& command, const CommandCompileContext& ctx)
        {
            eastl::array<DL::ITextureView*, MAX_RENDER_TARGETS_COUNT> renderTargets;
            ASSERT(MAX_RENDER_TARGETS_COUNT == command.desc.colorAttachments.size());

            uint32_t colorAttachmentCount = 0;
            for (uint32_t i = 0; i < command.desc.colorAttachments.size(); i++)
            {
                if (command.desc.colorAttachments[i].renderTargetView == nullptr)
                    continue;

                renderTargets[i] = command.desc.colorAttachments[i].renderTargetView->GetPrivateImpl<GpuResourceViewImpl>()->GetTextureView();
                colorAttachmentCount++;
            }

            DL::ITextureView* depthStencilView = nullptr;
            if (command.desc.depthStencilAttachment.depthStencilView)
                depthStencilView = command.desc.depthStencilAttachment.depthStencilView->GetPrivateImpl<GpuResourceViewImpl>()->GetTextureView();

            ctx.device->SetRenderTargets(colorAttachmentCount, renderTargets.data(), depthStencilView, DL::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            for (uint32_t i = 0; i < colorAttachmentCount; i++)
            {
                if (command.desc.colorAttachments[i].renderTargetView == nullptr)
                    continue;

                if (command.desc.colorAttachments[i].loadOp == AttachmentLoadOp::Clear)
                {
                    ctx.device->ClearRenderTarget(renderTargets[i], &command.desc.colorAttachments[i].clearColor, DL::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                }
            }

            if ((depthStencilView != nullptr) && command.desc.depthStencilAttachment.loadOp == AttachmentLoadOp::Clear)
            {
                ASSERT(command.desc.depthStencilAttachment.clearFlags != DepthStencilClearFlags::None);
                DL::CLEAR_DEPTH_STENCIL_FLAGS clearFlags = DL::CLEAR_DEPTH_FLAG_NONE;

                if (IsSet(command.desc.depthStencilAttachment.clearFlags, DepthStencilClearFlags::Depth))
                    clearFlags |= DL::CLEAR_DEPTH_FLAG;

                if (IsSet(command.desc.depthStencilAttachment.clearFlags, DepthStencilClearFlags::Stencil))
                    clearFlags |= DL::CLEAR_STENCIL_FLAG;

                ctx.device->ClearDepthStencil(
                    depthStencilView,
                    clearFlags,
                    command.desc.depthStencilAttachment.depthClearValue,
                    command.desc.depthStencilAttachment.stencilClearValue,
                    DL::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            }
            else
                ASSERT(command.desc.depthStencilAttachment.clearFlags == DepthStencilClearFlags::None);
        }

        // ---------------------------------------------------------------------------------------------
        // Draw commands
        // ---------------------------------------------------------------------------------------------

        template <typename T>
        void compileCommand(const T& command, bool indexed, const CommandCompileContext& ctx)
        {
            const auto* pso = static_cast<const PipelineStateImpl*>(command.psoImpl);
            ASSERT(pso);
            ASSERT(command.geometryLayout);

            ctx.device->SetPipelineState(pso->GetPipelineState());

            static constexpr size_t MAX_VERTEX_BINDINGS_COUNT = 8;
            eastl::array<DL::IBuffer*, MAX_VERTEX_BINDINGS_COUNT> vertexBuffers;
            eastl::array<uint64_t, MAX_VERTEX_BINDINGS_COUNT> vertexBufferOffsets;

            size_t vertexBindingsCount = command.geometryLayout->vertexBindings.size();
            ASSERT(vertexBindingsCount <= MAX_VERTEX_BINDINGS_COUNT);

            if (vertexBindingsCount > MAX_VERTEX_BINDINGS_COUNT)
            {
                static bool IsFirstTime = true;
                if (IsFirstTime)
                {
                    IsFirstTime = false;
                    Log::Format::Error("ERROR: Too many vertex buffers bound");
                }
                return;
            }

            for (size_t i = 0; i < vertexBindingsCount; i++)
            {
                auto vertexBinding = command.geometryLayout->vertexBindings[i];
                vertexBuffers[i] = vertexBinding.vertexBuffer ? vertexBinding.vertexBuffer->template GetPrivateImpl<GpuResourceImpl>()->GetAsBuffer() : nullptr;
                vertexBufferOffsets[i] = vertexBinding.vertexBufferOffset;
            }
            ctx.device->SetVertexBuffers(0, vertexBindingsCount, vertexBuffers.data(), vertexBufferOffsets.data(), DL::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            if (indexed)
            {
                const auto* indexBuffer = command.geometryLayout->indexBuffer;
                ASSERT(indexBuffer);
                ASSERT(indexBuffer->GetDesc().GetBufferMode() == GAPI::BufferMode::Formatted);
                ctx.device->SetIndexBuffer(indexBuffer->template GetPrivateImpl<GpuResourceImpl>()->GetAsBuffer(), 0, DL::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            }

            if (indexed)
            {
                const auto* indexBuffer = command.geometryLayout->indexBuffer;

                DL::DrawIndexedAttribs drawAttribs;
                drawAttribs.NumIndices = command.attribs.vertexCount;
                drawAttribs.FirstIndexLocation = command.attribs.startLocation;
                drawAttribs.NumInstances = Max(command.attribs.instanceCount, 1u);

                switch (indexBuffer->GetDesc().GetBufferFormat())
                {
                    case GAPI::GpuResourceFormat::R16Uint: drawAttribs.IndexType = DL::VT_UINT16; break;
                    case GAPI::GpuResourceFormat::R32Uint: drawAttribs.IndexType = DL::VT_UINT32; break;
                    default: ASSERT_MSG(false, "Unknown index buffer format"); break;
                }

                ctx.device->DrawIndexed(drawAttribs);
            }
            else
            {
                DL::DrawAttribs drawAttribs;
                drawAttribs.NumVertices = command.attribs.vertexCount;
                drawAttribs.StartVertexLocation = command.attribs.startLocation;
                drawAttribs.NumInstances = Max(command.attribs.instanceCount, 1u);

                ctx.device->Draw(drawAttribs);
            }
        }

        void compileCommand(const Commands::Draw& command, const CommandCompileContext& ctx)
        {
            compileCommand(command, false, ctx);
        }

        void compileCommand(const Commands::DrawIndexed& command, const CommandCompileContext& ctx)
        {
            compileCommand(command, true, ctx);
        }
    }

    CommandListImpl::~CommandListImpl() { }

    void CommandListImpl::Init() { }
    void CommandListImpl::Compile(GAPI::CommandList2& commandList, DL::IDeviceContext* device)
    {
        device->Begin(0);

        CommandCompileContext ctx{ device };

        ASSERT(commandList.size() != 0);

        for (const auto* command : commandList)
        {
            switch (command->type)
            {
            case Command::Type::SetRenderPass:
                compileCommand(static_cast<const Commands::SetRenderPass&>(*command), ctx);
                break;

            case Command::Type::Draw:
                compileCommand(static_cast<const Commands::Draw&>(*command), ctx);
                break;

            case Command::Type::DrawIndexed:
                compileCommand(static_cast<const Commands::DrawIndexed&>(*command), ctx);
                break;

            default:
                ASSERT_MSG(false, "Unknown command type");
                break;
            }
        }

        DL::ICommandList* commandListPtr;
        device->FinishCommandList(&commandListPtr);

        commandList.clear();

        diligentCommandList.Attach(commandListPtr);
    }

}
