#include "GpuResourceViews.hpp"

#include "gapi/Buffer.hpp"
#include "gapi/Texture.hpp"

namespace RR
{
    namespace GAPI
    {
        namespace
        {
            bool isCompatable(const GpuResourceViewDescription& desc, const GpuResourceDescription& resourceDesc)
            {
                bool result = resourceDesc.IsValid();

                // TODO actually  firstArraySlice should be compared with depth on 3d textures.
                if (resourceDesc.IsBuffer())
                {
                    result &= desc.buffer.firstElement + desc.buffer.elementCount <= resourceDesc.GetNumElements();
                }
                else
                {
                    result &= desc.texture.mipLevel + desc.texture.mipCount <= resourceDesc.texture.mipLevels;
                    result &= desc.texture.firstArraySlice + desc.texture.arraySliceCount <= resourceDesc.texture.arraySize;
                }

                return result;
            }

            void check(const GpuResourceViewDescription& desc, const std::weak_ptr<GpuResource>& gpuResource, GpuResourceBindFlags requiredBindFlags)
            {
                ASSERT(!gpuResource.expired());
                const auto sharedGpuResource = gpuResource.lock();
                const auto& resourceDescription = sharedGpuResource->GetDescription();

                ASSERT(isCompatable(desc, resourceDescription));
                ASSERT(IsSet(resourceDescription.bindFlags, requiredBindFlags));
            }
        }

        GpuResourceViewDescription::GpuResourceViewDescription(GpuResourceFormat format, uint32_t mipLevel, uint32_t mipsCount, uint32_t firstArraySlice, uint32_t arraySlicesCount)
            : texture({ mipLevel, mipsCount, firstArraySlice, arraySlicesCount }),
              format(format)
        {
            ASSERT(format != GpuResourceFormat::Unknown);
        }

        GpuResourceViewDescription::GpuResourceViewDescription(GpuResourceFormat format, uint32_t firstElement, uint32_t elementsCount)
            : buffer({ firstElement, elementsCount }),
              format(format)
        {
            ASSERT(format != GpuResourceFormat::Unknown);
        }

        ShaderResourceView::ShaderResourceView(
            const std::weak_ptr<GpuResource>& gpuResource,
            const GpuResourceViewDescription& desc)
            : GpuResourceView(GpuResourceView::ViewType::ShaderResourceView, gpuResource, desc)
        {
            ASSERT(!gpuResource.expired());
            check(desc, gpuResource, GpuResourceBindFlags::ShaderResource);
        }

        DepthStencilView::DepthStencilView(
            const std::weak_ptr<Texture>& texture,
            const GpuResourceViewDescription& desc)
            : GpuResourceView(GpuResourceView::ViewType::DepthStencilView, texture, desc)
        {
            ASSERT(!texture.expired());
            check(desc, texture, GpuResourceBindFlags::RenderTarget);
        }

        RenderTargetView::RenderTargetView(
            const std::weak_ptr<Texture>& texture,
            const GpuResourceViewDescription& desc)
            : GpuResourceView(GpuResourceView::ViewType::RenderTargetView, texture, desc)
        {
            ASSERT(!texture.expired());
            check(desc, texture, GpuResourceBindFlags::RenderTarget);
        }

        UnorderedAccessView::UnorderedAccessView(
            const std::weak_ptr<GpuResource>& gpuResource,
            const GpuResourceViewDescription& desc)
            : GpuResourceView(GpuResourceView::ViewType::UnorderedAccessView, gpuResource, desc)
        {
            ASSERT(!gpuResource.expired());
            check(desc, gpuResource, GpuResourceBindFlags::UnorderedAccess);
        }
    }
}