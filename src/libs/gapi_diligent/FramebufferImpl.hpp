#pragma once

#include "gapi/Framebuffer.hpp"

namespace Diligent {
    class IRenderDevice;
    class ITextureView;
}

namespace DL = Diligent;

namespace RR::GAPI::Diligent
{
    class FramebufferImpl final : public IFramebuffer
    {
    public:
        FramebufferImpl() = default;
        ~FramebufferImpl();

        void Init(GAPI::Framebuffer& resource);

        DL::ITextureView** GetRTVs() { return rtvs_.data(); }
        DL::ITextureView* GetRTV(uint32_t index) const { return rtvs_[index]; }
        DL::ITextureView* GetDSV() const { return dsv_; }

    private:
        eastl::array<DL::ITextureView*, Framebuffer::MaxRenderTargets> rtvs_;
        DL::ITextureView* dsv_;
    };
}