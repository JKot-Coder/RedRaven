#pragma once

//#include "gapi/Framebuffer.hpp"

namespace RR
{
    namespace GAPI::DX12
    {
        class FramebufferImpl final : public IFramebuffer
        {
        public:
            FramebufferImpl() = default;
            ~FramebufferImpl();

            void Init(const FramebufferDesc& description);

            uint32_t GetRTVDescriptiorsCount() const
            {
                ASSERT(inited_);
                return rtvDescriptorsCount_;
            }

            const D3D12_CPU_DESCRIPTOR_HANDLE* GetRTVDescriptiors() const
            {
                ASSERT(inited_);
                return rtvDescriptorsCount_ > 0 ? rtvDescriptors_.data() : nullptr;
            }

            const D3D12_CPU_DESCRIPTOR_HANDLE* GetDSVDescriptor() const
            {
                ASSERT(inited_);
                return depthStencil_ ? &dsvDescriptor_ : nullptr;
            }

        private:
            bool inited_ = false;
            bool depthStencil_;
            uint32_t rtvDescriptorsCount_;
            std::array<D3D12_CPU_DESCRIPTOR_HANDLE, Framebuffer::MaxRenderTargets> rtvDescriptors_;
            D3D12_CPU_DESCRIPTOR_HANDLE dsvDescriptor_;
        };
    }
}