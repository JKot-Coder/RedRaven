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

            if (sharedGpuResource->IsTexture())
            {
                const auto texture = sharedGpuResource->GetTyped<Texture>();
                const auto& description = texture->GetDescription();

                ASSERT(IsSet(description.GetBindFlags(), GpuResourceBindFlags::ShaderResource));
                ASSERT(desc.texture.mipLevel + desc.texture.mipCount <= description.GetMipCount());
                ASSERT(desc.texture.firstArraySlice + desc.texture.arraySliceCount <= description.GetArraySize());
            }
            else
            {
                ASSERT(false);
                // const auto buffer = sharedGpuResource->GetTyped<Buffer>();

                // ASSERT(desc.buffer.firstElement + desc.buffer.elementCount <= buffer->GetDescription().eme);\\
                ASSERT(IsSet(bindFlags, GpuResourceBindFlags::ShaderResource));
            }
        }

        DepthStencilView::DepthStencilView(
            const std::weak_ptr<Texture>& texture,
            const GpuResourceViewDescription& desc) : GpuResourceView(GpuResourceView::ViewType::DepthStencilView, texture, desc)
        {
            ASSERT(!texture.expired());

            const auto sharedTexutre = texture.lock();
            const auto& description = sharedTexutre->GetDescription();

            ASSERT(desc.texture.mipLevel + desc.texture.mipCount <= description.GetMipCount());
            ASSERT(desc.texture.firstArraySlice + desc.texture.arraySliceCount <= description.GetArraySize());
            ASSERT(IsSet(description.GetBindFlags(), GpuResourceBindFlags::RenderTarget));
        }

        RenderTargetView::RenderTargetView(
            const std::weak_ptr<Texture>& texture,
            const GpuResourceViewDescription& desc) : GpuResourceView(GpuResourceView::ViewType::RenderTargetView, texture, desc)
        {
            ASSERT(!texture.expired());

            const auto sharedTexutre = texture.lock();
            const auto& description = sharedTexutre->GetDescription();

            ASSERT(desc.texture.mipLevel + desc.texture.mipCount <= description.GetMipCount())
            ASSERT(desc.texture.firstArraySlice + desc.texture.arraySliceCount <= description.GetArraySize())
            ASSERT(IsSet(description.GetBindFlags(), GpuResourceBindFlags::RenderTarget));
        }

        UnorderedAccessView::UnorderedAccessView(
            const std::weak_ptr<GpuResource>& gpuResource,
            const GpuResourceViewDescription& desc) : GpuResourceView(GpuResourceView::ViewType::UnorderedAccessView, gpuResource, desc)
        {
            ASSERT(!gpuResource.expired());

            const auto sharedGpuResource = gpuResource.lock();

            if (sharedGpuResource->IsTexture())
            {
                const auto texture = sharedGpuResource->GetTyped<Texture>();
                const auto& description = texture->GetDescription();

                ASSERT(IsSet(description.GetBindFlags(), GpuResourceBindFlags::ShaderResource));
                ASSERT(desc.texture.mipLevel + desc.texture.mipCount <= description.GetMipCount());
                ASSERT(desc.texture.firstArraySlice + desc.texture.arraySliceCount <= description.GetArraySize());
            }
            else
            {
                ASSERT(false);
                // const auto buffer = sharedGpuResource->GetTyped<Buffer>();

                // ASSERT(desc.buffer.firstElement + desc.buffer.elementCount <= buffer->GetDescription().eme);
                //ASSERT(IsSet(sharedGpuResource->GetBindFlags(), GpuResourceBindFlags::ShaderResource));
            }
        }
    }
}