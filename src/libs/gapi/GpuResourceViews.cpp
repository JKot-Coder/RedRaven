#include "GpuResourceViews.hpp"

#include "gapi/Buffer.hpp"
#include "gapi/Texture.hpp"

namespace RR
{
    namespace GAPI
    {
        namespace
        {
            bool isCompatable(const GpuResourceViewDescription& desc, const TextureDescription& resourceDesc)
            {
                bool result = resourceDesc.IsValid();

                // TODO actually  firstArraySlice should be compared with depth on 3d textures.
                result &= desc.texture.mipLevel + desc.texture.mipCount <= resourceDesc.mipLevels;
                result &= desc.texture.firstArraySlice + desc.texture.arraySliceCount <= resourceDesc.arraySize;

                return result;
            }

            bool isCompatable(const GpuResourceViewDescription& desc, const BufferDescription& resourceDesc)
            {
                bool result = resourceDesc.IsValid();
                result &= desc.buffer.firstElement + desc.buffer.elementCount <= resourceDesc.GetNumElements();
                return result;
            }

            void check(const GpuResourceViewDescription& desc, const std::weak_ptr<GpuResource>& gpuResource, GpuResourceBindFlags requiredBindFlags)
            {
                ASSERT(!gpuResource.expired());
                const auto sharedGpuResource = gpuResource.lock();

                switch (sharedGpuResource->GetResourceType())
                {
                    case GpuResourceType::Buffer:
                    {
                        const auto buffer = sharedGpuResource->GetTyped<Buffer>();
                        ASSERT(isCompatable(desc, buffer->GetDescription()));
                        ASSERT(IsSet(buffer->GetDescription().bindFlags, requiredBindFlags));
                    }
                    break;
                    case GpuResourceType::Texture:
                    {
                        const auto texture = sharedGpuResource->GetTyped<Texture>();
                        ASSERT(isCompatable(desc, texture->GetDescription()));
                        ASSERT(IsSet(texture->GetDescription().bindFlags, requiredBindFlags));
                    }
                    break;
                    default: ASSERT_MSG(false, "Unknow resouce type");
                }
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