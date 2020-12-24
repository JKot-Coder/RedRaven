#include "CommandQueueImpl.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {

            CommandQueueImpl::CommandQueueImpl(D3D12_COMMAND_LIST_TYPE type) : type_(type)
            {
            }

            Result CommandQueueImpl::Init(const ComSharedPtr<ID3D12Device>& device, const U8String& name)
            {
                ASSERT(device)
                ASSERT(D3DCommandQueue_.get() == nullptr)

                D3D12_COMMAND_QUEUE_DESC desc = {};
                desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
                desc.Type = type_;

                D3DCallMsg(device->CreateCommandQueue(&desc, IID_PPV_ARGS(D3DCommandQueue_.put())), "CreateCommandQueue");
                D3DUtils::SetAPIName(D3DCommandQueue_.get(), name);

                return Result::Ok;
            }

            //    Result CommandQueueImpl::Submit(CommandListInterface& CommandContext)
            //  {
            //   }
        };
    }
}