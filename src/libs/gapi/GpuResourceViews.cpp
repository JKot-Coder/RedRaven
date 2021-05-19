#include "GpuResourceViews.hpp"

#include "gapi/Texture.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace
        {
            bool isCompatable(const GpuResourceViewDescription& desc, const GpuResourceDescription& resourceDesc)
            {
                bool result = resourceDesc.IsValid();

                if (resourceDesc.GetDimension() == GAPI::GpuResourceDimension::Buffer)
                {
                    result &= desc.buffer.firstElement + desc.buffer.elementCount <= resourceDesc.GetNumElements();
                } 
                else
                {
                    result &= desc.texture.mipLevel + desc.texture.mipCount <= resourceDesc.GetMipCount();
                    result &= desc.texture.firstArraySlice + desc.texture.arraySliceCount <= resourceDesc.GetArraySize();
                }          

                return result;
            }
        }

        ShaderResourceView::ShaderResourceView(
            const std::weak_ptr<GpuResource>& gpuResource,
            const GpuResourceViewDescription& desc) : GpuResourceView(GpuResourceView::ViewType::ShaderResourceView, gpuResource, desc)
        {
            ASSERT(!gpuResource.expired());

            const auto sharedGpuResource = gpuResource.lock();
            const auto& resoureDescription = sharedGpuResource->GetDescription();

            ASSERT(isCompatable(desc, resoureDescription));
            ASSERT(IsSet(resoureDescription.GetBindFlags(), GpuResourceBindFlags::ShaderResource));
        }

        DepthStencilView::DepthStencilView(
            const std::weak_ptr<Texture>& texture,
            const GpuResourceViewDescription& desc) : GpuResourceView(GpuResourceView::ViewType::DepthStencilView, texture, desc)
        {
            ASSERT(!texture.expired());

            const auto sharedGpuResource = texture.lock();
            const auto& resoureDescription = sharedGpuResource->GetDescription();

            ASSERT(isCompatable(desc, resoureDescription));
            ASSERT(resoureDescription.GetDimension() != GAPI::GpuResourceDimension::Buffer);
            ASSERT(IsSet(resoureDescription.GetBindFlags(), GpuResourceBindFlags::RenderTarget));
        }

        RenderTargetView::RenderTargetView(
            const std::weak_ptr<Texture>& texture,
            const GpuResourceViewDescription& desc) : GpuResourceView(GpuResourceView::ViewType::RenderTargetView, texture, desc)
        {
            ASSERT(!texture.expired());

            const auto sharedTexutre = texture.lock();
            const auto& resoureDescription = sharedTexutre->GetDescription();

            ASSERT(isCompatable(desc, resoureDescription));
            ASSERT(resoureDescription.GetDimension() != GAPI::GpuResourceDimension::Buffer);
            ASSERT(IsSet(resoureDescription.GetBindFlags(), GpuResourceBindFlags::RenderTarget));
        }

        UnorderedAccessView::UnorderedAccessView(
            const std::weak_ptr<GpuResource>& gpuResource,
            const GpuResourceViewDescription& desc) : GpuResourceView(GpuResourceView::ViewType::UnorderedAccessView, gpuResource, desc)
        {
            ASSERT(!gpuResource.expired());
            const auto sharedGpuResource = gpuResource.lock();
            const auto& resoureDescription = sharedGpuResource->GetDescription();

            ASSERT(isCompatable(desc, resoureDescription));
            ASSERT(IsSet(resoureDescription.GetBindFlags(), GpuResourceBindFlags::UnorderedAccess));
        }
    }
}