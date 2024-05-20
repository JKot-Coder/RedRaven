#include "CommandQueueImpl.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/Fence.hpp"

#include "gapi_dx12/CommandListImpl.hpp"
#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/InitialDataUploder.hpp"
#include "gapi_dx12/ResourceReleaseContext.hpp"

namespace RR
{
    namespace GAPI
    {
        namespace DX12
        {
            CommandQueueImpl::CommandQueueImpl(CommandQueueType type)
            {
                switch (type)
                {
                    case CommandQueueType::Graphics: type_ = D3D12_COMMAND_LIST_TYPE_DIRECT; break;
                    case CommandQueueType::Compute: type_ = D3D12_COMMAND_LIST_TYPE_COMPUTE; break;
                    case CommandQueueType::Copy: type_ = D3D12_COMMAND_LIST_TYPE_COPY; break;
                    default: LOG_FATAL("Unknown command queue type");
                }
            };

            CommandQueueImpl::~CommandQueueImpl()
            {
                ResourceReleaseContext::DeferredD3DResourceRelease(D3DCommandQueue_);
            }

            void CommandQueueImpl::ImmediateD3DObjectRelease()
            {
                D3DCommandQueue_ = nullptr;
            }

            void CommandQueueImpl::Init(const U8String& name)
            {
                ASSERT(!D3DCommandQueue_)

                const auto& device = DeviceContext::GetDevice();

                D3D12_COMMAND_QUEUE_DESC desc = {};
                desc.Type = type_;
                desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

                D3DCall(device->CreateCommandQueue(&desc, IID_PPV_ARGS(D3DCommandQueue_.put())));
                D3DUtils::SetAPIName(D3DCommandQueue_.get(), name);

                fence_ = std::make_shared<FenceImpl>();
                fence_->Init(name);
            }

            void CommandQueueImpl::Submit(CommandListImpl& commandList, bool waitForPendingUploads)
            {
                ASSERT(D3DCommandQueue_);
                ASSERT(type_ == commandList.GetType());

                if (waitForPendingUploads)
                    InitialDataUploder::Instance().FlushAndWaitFor(*this);

                std::array<ID3D12CommandList*, 2> commandLists;
                size_t numCommandLists = 0;

                auto pushCommandList = [&commandLists, &numCommandLists](const CommandListImpl& commandList) {
                    const auto& d3dCommandList = commandList.GetD3DObject();
                    ASSERT(d3dCommandList);

                    commandLists[numCommandLists] = d3dCommandList.get();
                    numCommandLists++;
                };

                pushCommandList(commandList);

                D3DCommandQueue_->ExecuteCommandLists(numCommandLists, commandLists.data());
                commandList.ResetAfterSubmit(*this);
            }

            void CommandQueueImpl::Submit(const std::shared_ptr<CommandList>& commandList)
            {
                ASSERT(commandList);

                const auto& commandListImpl = commandList->GetPrivateImpl<CommandListImpl>();
                ASSERT(commandListImpl);

                Submit(*commandListImpl, true);
            }

            void CommandQueueImpl::Signal(const FenceImpl& fence, uint64_t value)
            {
                ASSERT(D3DCommandQueue_);
                D3DCall(D3DCommandQueue_->Signal(fence.GetD3DObject().get(), value));
            }

            void CommandQueueImpl::Wait(const FenceImpl& fence, uint64_t value)
            {
                ASSERT(D3DCommandQueue_);
                D3DCall(D3DCommandQueue_->Wait(fence.GetD3DObject().get(), value));
            }

            void CommandQueueImpl::WaitForGpu()
            {
                ASSERT(fence_);
                fence_->Signal(*this);
                fence_->SyncCPU(std::nullopt);
            }
        };
    }
}