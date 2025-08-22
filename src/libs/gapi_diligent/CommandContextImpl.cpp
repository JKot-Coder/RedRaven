#include "CommandContextImpl.hpp"

#include "DeviceContext.h"

#include "GpuResourceViewImpl.hpp"
#include "gapi/Commands/Clear.hpp"

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
    }

    CommandContextImpl::~CommandContextImpl() { }

    void CommandContextImpl::Init() { }
    void CommandContextImpl::Compile(GAPI::CommandList2& commandList, DL::IDeviceContext* device)
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
            }
        }

        DL::ICommandList* commandListPtr;
        device->FinishCommandList(&commandListPtr);

        commandList.clear();

        this->commandList.Attach(commandListPtr);
    }

}
