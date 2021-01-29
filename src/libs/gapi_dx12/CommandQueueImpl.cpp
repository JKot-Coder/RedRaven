#include "CommandQueueImpl.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/Fence.hpp"

#include "gapi_dx12/CommandListImpl.hpp"
#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/ResourceReleaseContext.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            namespace
            {
                bool isListTypeCompatable(CommandQueueType commandQueueType, CommandListType commandListType)
                {
                    return (commandQueueType == CommandQueueType::Copy && commandListType == CommandListType::Copy) ||
                           (commandQueueType == CommandQueueType::Compute && commandListType == CommandListType::Compute) ||
                           (commandQueueType == CommandQueueType::Graphics && commandListType == CommandListType::Graphics);
                }
            }

            void CommandQueueImpl::ReleaseD3DObjects()
            {
                DeviceContext::GetResourceReleaseContext()->DeferredD3DResourceRelease(D3DCommandQueue_);
            }

            void CommandQueueImpl::Init(const U8String& name)
            {
                ASSERT(!D3DCommandQueue_)

                const auto& device = DeviceContext::GetDevice();

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
                    LOG_FATAL( "Unsuported command queue type");
                }

                D3DCall(device->CreateCommandQueue(&desc, IID_PPV_ARGS(D3DCommandQueue_.put())));
                D3DUtils::SetAPIName(D3DCommandQueue_.get(), name);
            }

            void CommandQueueImpl::Submit(const std::shared_ptr<CommandList>& commandList)
            {
                ASSERT(D3DCommandQueue_);
                ASSERT(commandList);
                ASSERT(isListTypeCompatable(type_, commandList->GetCommandListType()));

                const auto& commandListImpl = commandList->GetPrivateImpl<CommandListImpl>();
                ASSERT(commandListImpl);

                const auto& d3dCommandList = commandListImpl->GetD3DObject();
                ASSERT(d3dCommandList);

                ID3D12CommandList* commandLists[] = { d3dCommandList.get() };
                D3DCommandQueue_->ExecuteCommandLists(1, commandLists);

                commandListImpl->ResetAfterSubmit(*this);
            }

            void CommandQueueImpl::Signal(const ComSharedPtr<ID3D12Fence>& fence, uint64_t value)
            {
                ASSERT(D3DCommandQueue_);
                D3DCall(D3DCommandQueue_->Signal(fence.get(), value));
            }

            void CommandQueueImpl::Wait(const ComSharedPtr<ID3D12Fence>& fence, uint64_t value)
            {
                ASSERT(D3DCommandQueue_);
                D3DCall(D3DCommandQueue_->Wait(fence.get(), value));
            }
        };
    }
}