#pragma once

#include "common/Math.hpp"

#include "gapi/CommandList.hpp"

#include <queue>

namespace RR
{
    namespace Common
    {
        class IDataBuffer;
    }

    namespace GAPI::DX12
    {
        class FenceImpl;
        class CommandQueueImpl;
        class ResourceImpl;

        class CommandListImpl final : public ICommandList
        {
        public:
            CommandListImpl(const CommandListType commandListType);
            ~CommandListImpl();

            void Init(const U8String& name, int32_t index = -1);
            std::any GetNativeHandle() const override { return D3DCommandList_.get(); }
            D3D12_COMMAND_LIST_TYPE GetType() const { return type_; }
            void Close() override;

            // ---------------------------------------------------------------------------------------------
            // Copy command list
            // ---------------------------------------------------------------------------------------------

            void CopyGpuResource(const ResourceImpl& source, const ResourceImpl& dest);
            void CopyGpuResource(const std::shared_ptr<GpuResource>& source, const std::shared_ptr<GpuResource>& dest) override;
            void CopyBufferRegion(const std::shared_ptr<Buffer>& sourceBuffer, uint32_t sourceOffset,
                                  const std::shared_ptr<Buffer>& destBuffer, uint32_t destOffset, uint32_t numBytes) override;
            void CopyTextureSubresource(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx,
                                        const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx) override;
            void CopyTextureSubresourceRegion(const std::shared_ptr<Texture>& sourceTexture, uint32_t sourceSubresourceIdx, const Box3u& sourceBox,
                                              const std::shared_ptr<Texture>& destTexture, uint32_t destSubresourceIdx, const Vector3u& destPoint) override;

            void UpdateGpuResource(const std::shared_ptr<GpuResource>& resource, const std::shared_ptr<IDataBuffer>& resourceData);
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

            void SetFrameBuffer(const std::shared_ptr<Framebuffer>& framebuffer) override;
            void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) override;

            // ---------------------------------------------------------------------------------------------

            void ResetAfterSubmit(CommandQueueImpl& commandQueue);

            const ComSharedPtr<ID3D12GraphicsCommandList4>& GetD3DObject() const { return D3DCommandList_; }

        private:
            void copyIntermediate(const std::shared_ptr<GpuResource>& resource, const std::shared_ptr<CpuResourceData>& resourceData, bool readback) const;

            class CommandAllocatorsPool final
            {
            public:
                ~CommandAllocatorsPool();

                void Init(D3D12_COMMAND_LIST_TYPE type, const U8String& name);

                ComSharedPtr<ID3D12CommandAllocator> GetNextAllocator();
                void ResetAfterSubmit(CommandQueueImpl& commandQueue);

            private:
                using AllocatorFecnceValuePair = std::pair<ComSharedPtr<ID3D12CommandAllocator>, uint64_t>;
                ComSharedPtr<ID3D12CommandAllocator> createAllocator() const;

            private:
                U8String name_;
                D3D12_COMMAND_LIST_TYPE type_;
                std::unique_ptr<FenceImpl> fence_;
                std::queue<AllocatorFecnceValuePair> allocators_;
                uint32_t ringBufferIndex_ = 0;
            };

        private:
            // One frame executing on GPU
            static constexpr int InitialAllocatorsCount = MAX_GPU_FRAMES_BUFFERED + 1;

        private:
            D3D12_COMMAND_LIST_TYPE type_;

            // TODO do we need 4 version?
            ComSharedPtr<ID3D12GraphicsCommandList4> D3DCommandList_;
            CommandAllocatorsPool commandAllocatorsPool_;
        };
    };
}