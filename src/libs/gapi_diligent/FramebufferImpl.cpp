#include "FramebufferImpl.hpp"

#include "gapi/Framebuffer.hpp"
#include "gapi/GpuResourceViews.hpp"

#include "gapi_diligent/GpuResourceViewImpl.hpp"

namespace Diligent {
    class IRenderDevice;
    class Framebuffer;
}

namespace DL = Diligent;

namespace RR::GAPI::Diligent
{
    FramebufferImpl::~FramebufferImpl() { }

    void FramebufferImpl::Init(GAPI::Framebuffer& resource)
    {
        const auto& description = resource.GetDescription();

        for (uint32_t i = 0; i < description.renderTargetViews.size(); i++)
        {
            const auto* rtv = description.renderTargetViews[i];
            if (!rtv)
                continue;

            rtvs_[i] = rtv->GetPrivateImpl<GpuResourceViewImpl>()->GetTextureView();
        }

        if (description.depthStencilView)
            dsv_ = description.depthStencilView->GetPrivateImpl<GpuResourceViewImpl>()->GetTextureView();
    }
}