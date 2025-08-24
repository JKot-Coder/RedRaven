#include "CommandListImpl.hpp"

#include "DeviceContext.h"

#include "GpuResourceViewImpl.hpp"
#include "PipelineStateImpl.hpp"
#include "FramebufferImpl.hpp"
#include "gapi/commands/Clear.hpp"
#include "gapi/commands/Draw.hpp"

namespace DL = ::Diligent;

namespace RR::GAPI::Diligent
{
    namespace
    {
        struct CommandCompileContext
        {
            DL::IDeviceContext* device;
        };

        void compileCommand(const Commands::ClearRTV& command, const CommandCompileContext& ctx)
        {
            const auto* rtv = static_cast<const GpuResourceViewImpl*>(command.rtvImpl);
            ASSERT(rtv);

            ctx.device->ClearRenderTarget(rtv->GetTextureView(), &command.color, DL::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }

        void compileCommand(const Commands::ClearDSV& command, const CommandCompileContext& ctx)
        {
            const auto* dsv = static_cast<const GpuResourceViewImpl*>(command.dsvImpl);
            ASSERT(dsv);

            // TODO Stencil clear value;
            ctx.device->ClearDepthStencil(dsv->GetTextureView(), DL::CLEAR_DEPTH_FLAG, command.clearValue, 0, DL::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }

        void compileCommand(const Commands::Draw& command, const CommandCompileContext& ctx)
        {
            const auto* pso = static_cast<const PipelineStateImpl*>(command.psoImpl);
            ASSERT(pso);

            auto* framebuffer = static_cast<FramebufferImpl*>(command.framebufferImpl);
            ASSERT(framebuffer);

            ctx.device->SetRenderTargets(MAX_BACK_BUFFER_COUNT, framebuffer->GetRTVs(), framebuffer->GetDSV(), DL::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
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
            case Command::Type::ClearRenderTargetView:
                compileCommand(static_cast<const Commands::ClearRTV&>(*command), ctx);
                break;

            case Command::Type::ClearDepthStencilView:
                compileCommand(static_cast<const Commands::ClearDSV&>(*command), ctx);
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

        this->commandList.Attach(commandListPtr);
    }

}
