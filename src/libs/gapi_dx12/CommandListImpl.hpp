#pragma once

#include "common/Math.hpp"

#include "gapi/CommandList.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class FenceImpl;
            class CommandQueueImpl;

            class CommandListImpl final : public IGraphicsCommandList
            {
            public:
                CommandListImpl(const CommandListType commandListType);

                Result Init(const U8String& name);

                Result Close() override;

                void CopyBuffer(const std::shared_ptr<Buffer>& sourceBuffer, const std::shared_ptr<Buffer>& destBuffer) override;
                void CopyBufferRegion(const std::shared_ptr<Buffer>& sourceBuffer, uint32_t sourceOffset,
                                      const std::shared_ptr<Buffer>& destBuffer, uint32_t destOffset, uint32_t numBytes) override;

                void CopyTexture(const std::shared_ptr<Texture>& sourceTexture, const std::shared_ptr<Texture>& destTexture) override;
                void CopyTextureSubresource(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx,
                                            const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx) override;
                void CopyTextureSubresourceRegion(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx, const Box3u& sourceBox,
                                                  const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx, const Vector3u& destPoint) override;

                void UpdateTextureData(const std::shared_ptr<Texture>& texture, const std::vector<TextureSubresourceFootprint>& subresourceFootprint) override;
                void UpdateSubresourceData(const std::shared_ptr<Texture>& texture, uint32_t firstSubresource, const std::vector<TextureSubresourceFootprint>& subresourceFootprint) override;

                void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) override;

                void ReleaseD3DObjects();
                Result ResetAfterSubmit(CommandQueueImpl& commandQueue);

                const ComSharedPtr<ID3D12GraphicsCommandList>& GetD3DObject() const { return D3DCommandList_; }

            private:
                // One frame executing on GPU
                static constexpr int AllocatorsCount = MAX_GPU_FRAMES_BUFFERED + 1;

                class CommandAllocatorsPool
                {
                public:
                    Result Init(D3D12_COMMAND_LIST_TYPE type, const U8String& name);

                    void ReleaseD3DObjects();

                    const ComSharedPtr<ID3D12CommandAllocator>& GetNextAllocator();
                    Result ResetAfterSubmit(CommandQueueImpl& commandQueue);

                private:
                    struct AllocatorData final
                    {
                        uint64_t cpuFenceValue;
                        ComSharedPtr<ID3D12CommandAllocator> allocator;
                    };

                    Result createAllocator(
                        const U8String& name,
                        const uint32_t index,
                        ComSharedPtr<ID3D12CommandAllocator>& allocator) const;

                private:
                    U8String name_;
                    D3D12_COMMAND_LIST_TYPE type_;
                    std::unique_ptr<FenceImpl> fence_;
                    std::array<AllocatorData, AllocatorsCount> allocators_;
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