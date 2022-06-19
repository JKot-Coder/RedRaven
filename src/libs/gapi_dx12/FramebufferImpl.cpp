#include "FramebufferImpl.hpp"

#include "gapi_dx12/DescriptorHeap.hpp"
#include "gapi_dx12/DescriptorManager.hpp"

namespace RR::GAPI::DX12
{
    FramebufferImpl::~FramebufferImpl() { }

    void FramebufferImpl::Init(const FramebufferDescription& description)
    {
        ASSERT(!inited_);

        const auto& depthStencilView = description.depthStencilView;
        depthStencil_ = depthStencilView != nullptr;

        if (depthStencil_)
        {
            const auto allocation = depthStencilView->GetPrivateImpl<DescriptorHeap::Descriptor>();
            ASSERT(allocation);

            dsvDescriptor_ = allocation->GetCPUHandle();
        }

        const auto& nullRtv = DescriptorManager::Instance().GetNullRtvDescriptor(GpuResourceDimension::Texture2D);
        rtvDescriptorsCount_ = 0;
        for (size_t index = 0; index < description.renderTargetViews.size(); index++)
        {
            const auto& renderTargetView = description.renderTargetViews[index];

            if (!renderTargetView)
            {
                rtvDescriptors_[index] = nullRtv.GetCPUHandle();
                continue;
            }

            const auto allocation = renderTargetView->GetPrivateImpl<DescriptorHeap::Descriptor>();
            ASSERT(allocation);

            rtvDescriptors_[index] = allocation->GetCPUHandle();

            rtvDescriptorsCount_ = index + 1;
        }

        inited_ = true;
    }

}
