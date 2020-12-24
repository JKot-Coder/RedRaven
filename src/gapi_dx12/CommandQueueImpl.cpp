#include "CommandQueueImpl.hpp"

#include "gapi/CommandList.hpp"

#include "gapi_dx12/CommandListImpl.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {

            CommandQueueImpl::CommandQueueImpl(CommandQueueType type) : type_(type)
            {
            }

            Result CommandQueueImpl::Init(const ComSharedPtr<ID3D12Device>& device, const U8String& name)
            {
                ASSERT(device)
                ASSERT(D3DCommandQueue_.get() == nullptr)

                D3D12_COMMAND_QUEUE_DESC desc = {};
                desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

                switch (type_)
                {
                case CommandQueueType::Graphics:
                    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                    break;
                case CommandQueueType::Compute:
                    desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
                    break;
                case CommandQueueType::Copy:
                    desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
                    break;
                default:
                    ASSERT_MSG(false, "Unsuported command queue type");
                    return Result::NotImplemented;
                }

                D3DCallMsg(device->CreateCommandQueue(&desc, IID_PPV_ARGS(D3DCommandQueue_.put())), "CreateCommandQueue");
                D3DUtils::SetAPIName(D3DCommandQueue_.get(), name);

                return Result::Ok;
            }

            Result CommandQueueImpl::Submit(const std::shared_ptr<CommandList>& commandList)
            {
                ASSERT(D3DCommandQueue_);

                const auto commandListImpl = commandList->GetPrivateImpl<CommandListImpl>();
                ASSERT(commandListImpl);

                ID3D12CommandList* commandLists[] = { commandListImpl->GetD3DObject().get() };
                D3DCommandQueue_->ExecuteCommandLists(1, commandLists);

                return Result::Ok;
            }

        };
    }
}