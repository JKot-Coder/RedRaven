#include "CommandQueueImpl.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/Fence.hpp"

#include "gapi_dx12/CommandContextImpl.hpp"
#include "gapi_dx12/FenceImpl.hpp"

namespace OpenDemo
{
    namespace GAPI
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

                const auto& commandListImpl = commandList->GetPrivateImpl<CommandContextImpl>();
                ASSERT(commandListImpl);

                const auto& d3dCommandList = commandListImpl->GetD3DObject();
                ASSERT(d3dCommandList);

                ID3D12CommandList* commandLists[] = { d3dCommandList.get() };
                D3DCommandQueue_->ExecuteCommandLists(1, commandLists);

                return Result::Ok;
            }

            Result CommandQueueImpl::Signal(const std::shared_ptr<Fence>& fence, uint64_t value)
            {
                ASSERT(D3DCommandQueue_);

                const auto& fenceImpl = fence->GetPrivateImpl<FenceImpl>();
                ASSERT(fenceImpl);

                const auto& d3dFence = fenceImpl->GetD3DObject();
                ASSERT(d3dFence);

                D3DCallMsg(D3DCommandQueue_->Signal(d3dFence.get(), value), "Signal");

                return Result::Ok;
            }

            Result CommandQueueImpl::Wait(const ComSharedPtr<ID3D12Fence>& fence, uint64_t value)
            {
                ASSERT(D3DCommandQueue_);
                D3DCallMsg(D3DCommandQueue_->Wait(fence.get(), value), "Wait");

                return Result::Ok;
            }
        };
    }
}