#include "GpuResourceViews.hpp"

#include "gapi/GpuResource.hpp"
#include "gapi/Texture.hpp"

namespace RR
{
    namespace GAPI
    {
        namespace
        {
            bool isCompatable(const GpuResourceViewDesc& desc, const GpuResourceDesc& resourceDesc)
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

            void check(const GpuResourceViewDesc& desc, const GAPI::GpuResource& gpuResource, GpuResourceBindFlags requiredBindFlags)
            {
                #if ENABLE_ASSERTS
                    const auto& resourceDesc = gpuResource.GetDesc();

                    ASSERT(isCompatable(desc, resourceDesc));
                    ASSERT(IsSet(resourceDesc.bindFlags, requiredBindFlags));
                #else
                    UNUSED(desc, gpuResource, requiredBindFlags);
                #endif
            }
        }

        GpuResourceViewDesc::GpuResourceViewDesc(GpuResourceFormat format, uint32_t mipLevel, uint32_t mipsCount, uint32_t firstArraySlice, uint32_t arraySlicesCount)
            : texture({mipLevel, mipsCount, firstArraySlice, arraySlicesCount}),
              format(format)
        {
            ASSERT(format != GpuResourceFormat::Unknown);
        }

        GpuResourceViewDesc::GpuResourceViewDesc(GpuResourceFormat format, size_t firstElement, size_t elementsCount)
            : buffer({firstElement, elementsCount}),
              format(format)
        {
            ASSERT(format != GpuResourceFormat::Unknown);
            ASSERT(elementsCount > 0);
        }

        ShaderResourceView::ShaderResourceView(
            GAPI::GpuResource& gpuResource,
            const GpuResourceViewDesc& desc)
            : GpuResourceView(GpuResourceView::ViewType::ShaderResourceView, gpuResource, desc)
        {
            check(desc, gpuResource, GpuResourceBindFlags::ShaderResource);
        }

        DepthStencilView::DepthStencilView(
            GAPI::Texture& texture,
            const GpuResourceViewDesc& desc)
            : GpuResourceView(GpuResourceView::ViewType::DepthStencilView, texture, desc)
        {
            check(desc, texture, GpuResourceBindFlags::DepthStencil);
        }

        RenderTargetView::RenderTargetView(
            GAPI::Texture& texture,
            const GpuResourceViewDesc& desc)
            : GpuResourceView(GpuResourceView::ViewType::RenderTargetView, texture, desc)
        {;
            check(desc, texture, GpuResourceBindFlags::RenderTarget);
        }

        UnorderedAccessView::UnorderedAccessView(
            GAPI::GpuResource& gpuResource,
            const GpuResourceViewDesc& desc)
            : GpuResourceView(GpuResourceView::ViewType::UnorderedAccessView, gpuResource, desc)
        {
            check(desc, gpuResource, GpuResourceBindFlags::UnorderedAccess);
        }
    }
}