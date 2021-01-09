#include "CommandListImpl.hpp"

#include "gapi_dx12/ResourceReleaseContext.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            void CommandListImpl::ReleaseD3DObjects(ResourceReleaseContext& releaseContext)
            {
                releaseContext.DeferredD3DResourceRelease(D3DCommandList_);        
            }
            
            Result CommandListImpl::Init(const ComSharedPtr<ID3D12Device>& device, const U8String& name)
            {
                ASSERT(device)
                ASSERT(D3DCommandList_.get() == nullptr)

                const auto& commandListType = _type;

                auto newCommandAllocator = [name, &device, commandListType](int index, ID3D12CommandAllocator*& allocator) -> Result {
                    D3DCallMsg(device->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&allocator)), "CreateCommandAllocator");

                    D3DUtils::SetAPIName(allocator, name, index);

                    return Result::Ok;
                };

                _allocatorsRB.reset(new FencedFrameRingBuffer<ID3D12CommandAllocator*>());
                D3DCall(_allocatorsRB->Init(device, newCommandAllocator, fmt::format("CommandList::{}", name)));

                const auto& allocator = _allocatorsRB->CurrentObject();
                D3DCallMsg(device->CreateCommandList(0, commandListType, allocator, nullptr, IID_PPV_ARGS(D3DCommandList_.put())), "CreateCommandList");

                D3DUtils::SetAPIName(D3DCommandList_.get(), name);

                return Result::Ok;
            }

            Result CommandListImpl::Reset()
            {
                ASSERT(D3DCommandList_.get())
                const auto& allocator = _allocatorsRB->GetNextObject();

                D3DCall(allocator->Reset());
                D3DCall(D3DCommandList_->Reset(allocator, nullptr));
                //  D3DCall(_allocatorsRB->MoveToNextFrame(queue));

                return Result::Ok;
            }

            Result CommandListImpl::Submit(ID3D12CommandQueue* queue)
            {
                ASSERT(queue)
                ASSERT(D3DCommandList_.get())
                const auto& allocator = _allocatorsRB->GetNextObject();

                ID3D12CommandList* commandLists[] = { D3DCommandList_.get() };
                queue->ExecuteCommandLists(1, commandLists);

                D3DCall(allocator->Reset());
                D3DCall(D3DCommandList_->Reset(allocator, nullptr));
                D3DCall(_allocatorsRB->MoveToNextFrame(queue));

                return Result::Ok;
            }
        }
    }
}