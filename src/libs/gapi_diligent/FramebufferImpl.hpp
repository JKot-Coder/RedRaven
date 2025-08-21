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
    private:
        eastl::array<DL::ITextureView*, Framebuffer::MaxRenderTargets> rtvs_;
        DL::ITextureView* dsv_;
    };
}