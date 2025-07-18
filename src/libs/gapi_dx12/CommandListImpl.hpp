#pragma once

#include "math/Math.hpp"

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

            void Init(const std::string& name, int32_t index = -1);
            std::any GetNativeHandle() const override { return D3DCommandList_.get(); }
            D3D12_COMMAND_LIST_TYPE GetType() const { return type_; }

            void Reset() override;
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
            void UpdateGpuResource(const std::shared_ptr<GpuResource>& resource, const std::shared_ptr<Common::IDataBuffer>& resourceData);

            // ---------------------------------------------------------------------------------------------
            // Compute command list
            // ---------------------------------------------------------------------------------------------

            void ClearUnorderedAccessViewUint(const std::shared_ptr<UnorderedAccessView>& unorderedAcessView, const Vector4u& clearValue) override;
            void ClearUnorderedAccessViewFloat(const std::shared_ptr<UnorderedAccessView>& unorderedAcessView, const Vector4& clearValue) override;

            // ---------------------------------------------------------------------------------------------
            // Graphics command list
            // ---------------------------------------------------------------------------------------------

            void SetFrameBuffer(const std::shared_ptr<Framebuffer>& framebuffer) override;
            void SetIndexBuffer(const std::shared_ptr<Buffer>& buffer, size_t offset = 0) override;
            void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) override;

            // ---------------------------------------------------------------------------------------------

            FenceImpl& GetSubmissionFence() const;

            const ComSharedPtr<ID3D12GraphicsCommandList4>& GetD3DObject() const { return D3DCommandList_; }

        private:
            class CommandAllocatorsPool final
            {
            public:
                ~CommandAllocatorsPool();

                void Init(D3D12_COMMAND_LIST_TYPE type, const std::string& name);

                ComSharedPtr<ID3D12CommandAllocator> GetNextAllocator();
                FenceImpl& GetSubmissionFence() const { return *fence_; }

            private:
                using AllocatorFecnceValuePair = std::pair<ComSharedPtr<ID3D12CommandAllocator>, uint64_t>;
                ComSharedPtr<ID3D12CommandAllocator> createAllocator() const;

            private:
                std::string name_;
                D3D12_COMMAND_LIST_TYPE type_;
                std::unique_ptr<FenceImpl> fence_;
                std::queue<AllocatorFecnceValuePair> allocators_;
                // uint32_t ringBufferIndex_ = 0;
            };

        private:
            // One frame executing on GPU
            static constexpr int InitialAllocatorsCount = MAX_GPU_FRAMES_BUFFERED + 1;
            static constexpr int MaxAllocatorsCount = 16;

        private:
            D3D12_COMMAND_LIST_TYPE type_;

            // TODO do we need 4 version?
            ComSharedPtr<ID3D12GraphicsCommandList4> D3DCommandList_;
            CommandAllocatorsPool commandAllocatorsPool_;
        };
    };
}