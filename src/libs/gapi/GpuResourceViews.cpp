#include "GpuResourceViews.hpp"

#include "gapi/Texture.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        ShaderResourceView::ShaderResourceView(
            const std::weak_ptr<GpuResource>& gpuResource,
            const GpuResourceViewDescription& desc,
            const U8String& name) : GpuResourceView(GpuResourceView::ViewType::ShaderResourceView, gpuResource, desc, name)
        {
            ASSERT(!gpuResource.expired());

            const auto sharedGpuResource = gpuResource.lock();

            if (sharedGpuResource->GetGpuResourceType() == GpuResource::Type::Texture)
            {
                const auto texture = sharedGpuResource->GetTyped<Texture>();

                ASSERT(desc.texture.mipLevel + desc.texture.mipCount <= texture->GetDescription().mipLevels);
                ASSERT(desc.texture.firstArraySlice + desc.texture.arraySliceCount <= texture->GetDescription().arraySize);
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
            const GpuResourceViewDescription& desc,
            const U8String& name) : GpuResourceView(GpuResourceView::ViewType::DepthStencilView, texture, desc, name)
        {
            ASSERT(!texture.expired());

            const auto sharedTexutre = texture.lock();

            ASSERT(desc.texture.mipLevel + desc.texture.mipCount <= sharedTexutre->GetDescription().mipLevels);
            ASSERT(desc.texture.firstArraySlice + desc.texture.arraySliceCount <= sharedTexutre->GetDescription().arraySize);
            ASSERT(IsSet(sharedTexutre->GetBindFlags(), GpuResourceBindFlags::RenderTarget));
        }

        RenderTargetView::RenderTargetView(
            const std::weak_ptr<Texture>& texture,
            const GpuResourceViewDescription& desc,
            const U8String& name) : GpuResourceView(GpuResourceView::ViewType::RenderTargetView, texture, desc, name)
        {
            ASSERT(!texture.expired());

            const auto sharedTexutre = texture.lock();

            ASSERT(desc.texture.mipLevel + desc.texture.mipCount <= sharedTexutre->GetDescription().mipLevels)
            ASSERT(desc.texture.firstArraySlice + desc.texture.arraySliceCount <= sharedTexutre->GetDescription().arraySize)
            ASSERT(IsSet(sharedTexutre->GetBindFlags(), GpuResourceBindFlags::RenderTarget));
        }

        UnorderedAccessView::UnorderedAccessView(
            const std::weak_ptr<GpuResource>& gpuResource,
            const GpuResourceViewDescription& desc,
            const U8String& name) : GpuResourceView(GpuResourceView::ViewType::UnorderedAccessView, gpuResource, desc, name)
        {
            ASSERT(!gpuResource.expired());

            const auto sharedGpuResource = gpuResource.lock();

            if (sharedGpuResource->GetGpuResourceType() == GpuResource::Type::Texture)
            {
                const auto texture = sharedGpuResource->GetTyped<Texture>();

                ASSERT(desc.texture.mipLevel + desc.texture.mipCount <= texture->GetDescription().mipLevels);
                ASSERT(desc.texture.firstArraySlice + desc.texture.arraySliceCount <= texture->GetDescription().arraySize);
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