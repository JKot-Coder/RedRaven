#pragma once

#include "common/Math.hpp"

#include "gapi/CommandList.hpp"

namespace RR
{
    namespace GAPI
    {
        namespace DX12
        {
            class FenceImpl;
            class CommandQueueImpl;

            class CommandListImpl final : public ICommandList
            {
            public:
                CommandListImpl(const CommandListType commandListType);
                ~CommandListImpl();

                void Init(const U8String& name);
                std::any GetNativeHandle() const override { return D3DCommandList_.get(); }
                void Close() override;

                // ---------------------------------------------------------------------------------------------
                // Copy command list
                // ---------------------------------------------------------------------------------------------

                void CopyGpuResource(const std::shared_ptr<GpuResource>& source, const std::shared_ptr<GpuResource>& dest) override;
                void CopyBufferRegion(const std::shared_ptr<Buffer>& sourceBuffer, uint32_t sourceOffset,
                                      const std::shared_ptr<Buffer>& destBuffer, uint32_t destOffset, uint32_t numBytes) override;
                void CopyTextureSubresource(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx,
                                            const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx) override;
                void CopyTextureSubresourceRegion(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx, const Box3u& sourceBox,
                                                  const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx, const Vector3u& destPoint) override;

                void UpdateGpuResource(const std::shared_ptr<GpuResource>& resource, const std::shared_ptr<CpuResourceData>& resourceData) override;
                void ReadbackGpuResource(const std::shared_ptr<GpuResource>& texture, const std::shared_ptr<CpuResourceData>& textureData) override;

                // ---------------------------------------------------------------------------------------------
                // Compute command list
                // ---------------------------------------------------------------------------------------------

                void ClearUnorderedAccessViewUint(const std::shared_ptr<UnorderedAccessView>& unorderedAcessView, const Vector4u& clearValue) override;
                void ClearUnorderedAccessViewFloat(const std::shared_ptr<UnorderedAccessView>& unorderedAcessView, const Vector4& clearValue) override;

                // ---------------------------------------------------------------------------------------------
                // Graphics command list
                // ---------------------------------------------------------------------------------------------

                void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) override;

                // ---------------------------------------------------------------------------------------------

                void ResetAfterSubmit(CommandQueueImpl& commandQueue);

                const ComSharedPtr<ID3D12GraphicsCommandList>& GetD3DObject() const { return D3DCommandList_; }

            private:
                // One frame executing on GPU
                // TODO allow dynamic allocators list size.
                static constexpr int AllocatorsCount = MAX_GPU_FRAMES_BUFFERED + 1;

                void copyIntermediate(const std::shared_ptr<GpuResource>& resource, const std::shared_ptr<CpuResourceData>& resourceData, bool readback) const;

                class CommandAllocatorsPool final
                {
                public:
                    ~CommandAllocatorsPool();

                    void Init(D3D12_COMMAND_LIST_TYPE type, const U8String& name);

                    const ComSharedPtr<ID3D12CommandAllocator>& GetNextAllocator();
                    void ResetAfterSubmit(CommandQueueImpl& commandQueue);

                private:
                    struct AllocatorData final
                    {
                        uint64_t cpuFenceValue;
                        ComSharedPtr<ID3D12CommandAllocator> allocator;
                    };

                    void createAllocator(
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