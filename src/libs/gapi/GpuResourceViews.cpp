#include "GpuResourceViews.hpp"

#include "gapi/Texture.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        ShaderResourceView::ShaderResourceView(
            const std::weak_ptr<GpuResource>& gpuResource,
            const GpuResourceViewDescription& desc) : GpuResourceView(GpuResourceView::ViewType::ShaderResourceView, gpuResource, desc)
        {
            ASSERT(!gpuResource.expired());

            const auto sharedGpuResource = gpuResource.lock();

            if (sharedGpuResource->GetGpuResourceType() == GpuResource::Type::Texture)
            {
                const auto texture = sharedGpuResource->GetTyped<Texture>();

                ASSERT(desc.texture.mipLevel + desc.texture.mipCount <= texture->GetDescription().GetMipCount());
                ASSERT(desc.texture.firstArraySlice + desc.texture.arraySliceCount <= texture->GetDescription().GetArraySize());
            }
            else
            {
                ASSERT(false);
                // const auto buffer = sharedGpuResource->GetTyped<Buffer>();

                // ASSERT(desc.buffer.firstElement + desc.buffer.elementCount <= buffer->GetDescription().eme);
            }

            ASSERT(IsSet(sharedGpuResource->GetBindFlags(), GpuResourceBindFlags::ShaderResource));
        }

        DepthStencilView::DepthStencilView(
            const std::weak_ptr<Texture>& texture,
            const GpuResourceViewDescription& desc) : GpuResourceView(GpuResourceView::ViewType::DepthStencilView, texture, desc)
        {
            ASSERT(!texture.expired());

            const auto sharedTexutre = texture.lock();

            ASSERT(desc.texture.mipLevel + desc.texture.mipCount <= sharedTexutre->GetDescription().GetMipCount());
            ASSERT(desc.texture.firstArraySlice + desc.texture.arraySliceCount <= sharedTexutre->GetDescription().GetArraySize());
            ASSERT(IsSet(sharedTexutre->GetBindFlags(), GpuResourceBindFlags::RenderTarget));
        }

        RenderTargetView::RenderTargetView(
            const std::weak_ptr<Texture>& texture,
            const GpuResourceViewDescription& desc) : GpuResourceView(GpuResourceView::ViewType::RenderTargetView, texture, desc)
        {
            ASSERT(!texture.expired());

            const auto sharedTexutre = texture.lock();

            ASSERT(desc.texture.mipLevel + desc.texture.mipCount <= sharedTexutre->GetDescription().GetMipCount())
            ASSERT(desc.texture.firstArraySlice + desc.texture.arraySliceCount <= sharedTexutre->GetDescription().GetArraySize())
            ASSERT(IsSet(sharedTexutre->GetBindFlags(), GpuResourceBindFlags::RenderTarget));
        }

        UnorderedAccessView::UnorderedAccessView(
            const std::weak_ptr<GpuResource>& gpuResource,
            const GpuResourceViewDescription& desc) : GpuResourceView(GpuResourceView::ViewType::UnorderedAccessView, gpuResource, desc)
        {
            ASSERT(!gpuResource.expired());

            const auto sharedGpuResource = gpuResource.lock();

            if (sharedGpuResource->GetGpuResourceType() == GpuResource::Type::Texture)
            {
                const auto texture = sharedGpuResource->GetTyped<Texture>();

                ASSERT(desc.texture.mipLevel + desc.texture.mipCount <= texture->GetDescription().GetMipCount());
                ASSERT(desc.texture.firstArraySlice + desc.texture.arraySliceCount <= texture->GetDescription().GetArraySize());
            }
            else
            {
                ASSERT(false);
                // const auto buffer = sharedGpuResource->GetTyped<Buffer>();

                // ASSERT(desc.buffer.firstElement + desc.buffer.elementCount <= buffer->GetDescription().eme);
            }

            ASSERT(IsSet(sharedGpuResource->GetBindFlags(), GpuResourceBindFlags::ShaderResource));
        }
    }
}