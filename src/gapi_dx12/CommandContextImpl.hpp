#pragma once

#include "common/Math.hpp"

#include "gapi/CommandList.hpp"

#include "gapi_dx12/CommandListImpl.hpp"

#include <queue>

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class ResourceReleaseContext;

            class CommandContextImpl final : public IGraphicsCommandList
            {
            public:
                CommandContextImpl(const CommandListType commandListType);

                void ReleaseD3DObjects(ResourceReleaseContext& releaseContext);

                Result Init(const ComSharedPtr<ID3D12Device>& device, const U8String& name);

                Result Close() override;
                Result ResetAfterSubmit(CommandQueueImpl& commandQueue);

                void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) override;

                const ComSharedPtr<ID3D12GraphicsCommandList>& GetD3DObject() const { return D3DCommandList_; }

            private:
                class CommandAllocatorsPool
                {
                public:
                    Result Init(const ComSharedPtr<ID3D12Device>& device, D3D12_COMMAND_LIST_TYPE type, const U8String& name);

                    void ReleaseD3DObjects(ResourceReleaseContext& releaseContext);

                    const ComSharedPtr<ID3D12CommandAllocator>& GetNextAllocator();
                    Result ResetAfterSubmit(CommandQueueImpl& commandQueue);
                private:
                    struct AllocatorData final
                    {
                        uint64_t cpuFenceValue;
                        ComSharedPtr<ID3D12CommandAllocator> allocator;
                    };

                    Result createAllocator(
                        const ComSharedPtr<ID3D12Device>& device,
                        ComSharedPtr<ID3D12CommandAllocator>& allocator) const;

                private:
                    U8String name_;
                    D3D12_COMMAND_LIST_TYPE type_;
                    std::unique_ptr<FenceImpl> fence_;
                    std::array<AllocatorData, MAX_GPU_FRAMES_BUFFERED> allocators_;
                    uint32_t ringBufferIndex_ = 0;
                };

            private:
                D3D12_COMMAND_LIST_TYPE type_;
                ComSharedPtr<ID3D12GraphicsCommandList> D3DCommandList_;
                CommandAllocatorsPool commandAllocatorsPool_;
            };
        };
    }
}