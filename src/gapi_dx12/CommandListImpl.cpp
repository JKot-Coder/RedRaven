#include "CommandListImpl.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace Device
        {
            namespace DX12
            {

                GAPIStatus CommandListImpl::Init(ID3D12Device* device, const U8String& name)
                {
                    ASSERT(device)
                    ASSERT(_commandList.get() == nullptr)

                    GAPIStatus result = GAPIStatus::OK;
                    const auto& commandListType = _type;

                    auto newCommandAllocator = [name, device, commandListType](int index) -> ID3D12CommandAllocator* {
                        ID3D12CommandAllocator* allocator;
                        if (FAILED(device->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&allocator))))
                        {
                            Log::Print::Error("Failed to create command allocator");
                            return nullptr;
                        }

                        D3DUtils::SetAPIName(allocator, name, index);

                        return allocator;
                    };

                    _allocatorsRB.reset(new FencedFrameRingBuffer<ID3D12CommandAllocator*>());
                    if (GAPIStatusU::Failure(result = _allocatorsRB->Init(device, newCommandAllocator, fmt::format("CommandList::{}", name))))
                    {
                        LOG_ERROR("Failure create FencedFrameRingBuffer with HRESULT of 0x%08X", result);
                        return result;
                    }

                    const auto& allocator = _allocatorsRB->CurrentObject();
                    if (GAPIStatusU::Failure(result = GAPIStatus(device->CreateCommandList(0, commandListType, allocator, nullptr, IID_PPV_ARGS(_commandList.put())))))
                    {
                        LOG_ERROR("Failure create CreateFence with HRESULT of 0x%08X", result);
                        return result;
                    }

                    D3DUtils::SetAPIName(_commandList.get(), name);

                    return result;
                }

                GAPIStatus CommandListImpl::Submit(ID3D12CommandQueue* queue)
                {
                    ASSERT(queue)
                    ASSERT(_commandList.get())

                    const auto& allocator = _allocatorsRB->GetNextObject();
                    GAPIStatus result = GAPIStatus::OK;

                    ID3D12CommandList* commandLists[] = { _commandList.get() };
                    queue->ExecuteCommandLists(1, commandLists);

                    if (GAPIStatusU::Failure(result = GAPIStatus(allocator->Reset())))
                        return result;

                    if (GAPIStatusU::Failure(result = GAPIStatus(_commandList->Reset(allocator, nullptr))))
                        return result;

                    if (GAPIStatusU::Failure(result = _allocatorsRB->MoveToNextFrame(queue)))
                        return result;
                }
            }
        }
    }
}