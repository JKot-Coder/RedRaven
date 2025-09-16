#include "CommandListImpl.hpp"

#include "DeviceContext.h"

#include "GpuResourceViewImpl.hpp"
#include "PipelineStateImpl.hpp"

#include "gapi/RenderPassDesc.hpp"

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

        void compileCommand(const Commands::Draw& command, const CommandCompileContext& ctx)
        {
            const auto* pso = static_cast<const PipelineStateImpl*>(command.psoImpl);
            ASSERT(pso);

            ctx.device->SetPipelineState(pso->GetPipelineState());

            DL::DrawAttribs drawAttribs;
            drawAttribs.NumVertices = command.attribs.vertexCount;
            drawAttribs.StartVertexLocation = command.attribs.startVertex;
            drawAttribs.NumInstances = Max(command.attribs.instanceCount, 1u);

            ctx.device->Draw(drawAttribs);
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
