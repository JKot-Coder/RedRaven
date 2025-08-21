#include "GpuResourceViews.hpp"

#include "gapi/GpuResource.hpp"
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

                if (resourceDesc.IsBuffer())
                {
                    result &= desc.buffer.firstElement + desc.buffer.elementCount <= resourceDesc.buffer.size;
                }
                else
                {
                    result &= desc.texture.mipLevel + desc.texture.mipCount <= resourceDesc.texture.mipLevels;
                    // TODO actually  firstArraySlice should be compared with depth on 3d textures.
                    result &= desc.texture.firstArraySlice + desc.texture.arraySliceCount <= resourceDesc.texture.arraySize;
                }

                return result;
            }

            void check(const GpuResourceViewDescription& desc, const eastl::shared_ptr<GpuResource>& gpuResource, GpuResourceBindFlags requiredBindFlags)
            {
                ASSERT(gpuResource);
                #if ENABLE_ASSERTS
                    const auto& resourceDescription = gpuResource->GetDescription();

                    ASSERT(isCompatable(desc, resourceDescription));
                    ASSERT(IsSet(resourceDescription.bindFlags, requiredBindFlags));
                #else
                    UNUSED(desc, gpuResource, requiredBindFlags);
                #endif
            }
        }

        GpuResourceViewDescription::GpuResourceViewDescription(GpuResourceFormat format, uint32_t mipLevel, uint32_t mipsCount, uint32_t firstArraySlice, uint32_t arraySlicesCount)
            : texture({mipLevel, mipsCount, firstArraySlice, arraySlicesCount}),
              format(format)
        {
            ASSERT(format != GpuResourceFormat::Unknown);
        }

        GpuResourceViewDescription::GpuResourceViewDescription(GpuResourceFormat format, size_t firstElement, size_t elementsCount)
            : buffer({firstElement, elementsCount}),
              format(format)
        {
            ASSERT(format != GpuResourceFormat::Unknown);
            ASSERT(elementsCount > 0);
        }

        ShaderResourceView::ShaderResourceView(
            const eastl::shared_ptr<GpuResource>& gpuResource,
            const GpuResourceViewDescription& desc)
            : GpuResourceView(GpuResourceView::ViewType::ShaderResourceView, gpuResource, desc)
        {
            check(desc, gpuResource, GpuResourceBindFlags::ShaderResource);
        }

        DepthStencilView::DepthStencilView(
            const eastl::shared_ptr<Texture>& texture,
            const GpuResourceViewDescription& desc)
            : GpuResourceView(GpuResourceView::ViewType::DepthStencilView, texture, desc)
        {
            check(desc, texture, GpuResourceBindFlags::RenderTarget);
        }

        RenderTargetView::RenderTargetView(
            const eastl::shared_ptr<Texture>& texture,
            const GpuResourceViewDescription& desc)
            : GpuResourceView(GpuResourceView::ViewType::RenderTargetView, texture, desc)
        {;
            check(desc, texture, GpuResourceBindFlags::RenderTarget);
        }

        UnorderedAccessView::UnorderedAccessView(
            const eastl::shared_ptr<GpuResource>& gpuResource,
            const GpuResourceViewDescription& desc)
            : GpuResourceView(GpuResourceView::ViewType::UnorderedAccessView, gpuResource, desc)
        {
            check(desc, gpuResource, GpuResourceBindFlags::UnorderedAccess);
        }
    }
}