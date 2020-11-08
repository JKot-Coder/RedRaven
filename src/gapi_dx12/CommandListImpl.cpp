#include "CommandListImpl.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {

            Result CommandListImpl::Init(ID3D12Device* device, const U8String& name)
            {
                ASSERT(device)
                ASSERT(_commandList.get() == nullptr)

                const auto& commandListType = _type;

                auto newCommandAllocator = [name, device, commandListType](int index, ID3D12CommandAllocator* &allocator) -> Result {
                    D3DCallMsg(device->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&allocator)), "CreateCommandAllocator");

                    D3DUtils::SetAPIName(allocator, name, index);

                    return Result::Ok;
                };

                _allocatorsRB.reset(new FencedFrameRingBuffer<ID3D12CommandAllocator*>());
                D3DCall(_allocatorsRB->Init(device, newCommandAllocator, fmt::format("CommandList::{}", name)));

                const auto& allocator = _allocatorsRB->CurrentObject();
                D3DCallMsg(device->CreateCommandList(0, commandListType, allocator, nullptr, IID_PPV_ARGS(_commandList.put())), "CreateCommandList");

                D3DUtils::SetAPIName(_commandList.get(), name);

                return Result::Ok;
            }

            Result CommandListImpl::Submit(ID3D12CommandQueue* queue)
            {
                ASSERT(queue)
                ASSERT(_commandList.get())
                const auto& allocator = _allocatorsRB->GetNextObject();

                ID3D12CommandList* commandLists[] = { _commandList.get() };
                queue->ExecuteCommandLists(1, commandLists);

                D3DCall(allocator->Reset());
                D3DCall(_commandList->Reset(allocator, nullptr));
                D3DCall(_allocatorsRB->MoveToNextFrame(queue));

                return Result::Ok;
            }
        }
    }
}