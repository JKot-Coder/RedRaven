#include "TextureViewImpl.hpp"

#include "TextureImpl.hpp"
#include "Utils.hpp"

namespace RR::GAPI::WebGPU
{
    TextureViewImpl::~TextureViewImpl() { }

    wgpu::TextureViewDescriptor getTextureViewDesc(GAPI::GpuResourceView::ViewType viewType, const GpuResourceViewDesc& desc)
    {
        auto getUsage = [](GAPI::GpuResourceView::ViewType viewType) -> wgpu::TextureUsage {
            switch (viewType)
            {
                case GAPI::GpuResourceView::ViewType::ShaderResourceView: return wgpu::TextureUsage::TextureBinding;
                case GAPI::GpuResourceView::ViewType::DepthStencilView: return wgpu::TextureUsage::RenderAttachment;
                case GAPI::GpuResourceView::ViewType::RenderTargetView: return wgpu::TextureUsage::CopyDst;
                case GAPI::GpuResourceView::ViewType::UnorderedAccessView: return wgpu::TextureUsage::StorageBinding;
                default:
                    ASSERT_MSG(false, "Unknown view type");
                    return wgpu::TextureUsage::None;
            }
        };

        wgpu::TextureViewDescriptor viewDesc;
        viewDesc.setDefault();
        viewDesc.format = GetWGPUFormat(desc.format);
        viewDesc.baseMipLevel = desc.texture.mipLevel;
        viewDesc.mipLevelCount = desc.texture.mipCount;
        viewDesc.baseArrayLayer = desc.texture.firstArraySlice;
        viewDesc.arrayLayerCount = desc.texture.arraySliceCount;
        viewDesc.aspect = wgpu::TextureAspect::All;
        viewDesc.usage = getUsage(viewType);
        return viewDesc;
    }

    void TextureViewImpl::Init(GAPI::GpuResourceView& gpuResourceView)
    {
        const auto gpuResource = gpuResourceView.GetGpuResource().lock();
        ASSERT(gpuResource);
        ASSERT(gpuResource->GetDesc().IsTexture());

        const TextureImpl* textureImpl = gpuResource->GetPrivateImpl<TextureImpl>();
        view = textureImpl->CreateView(getTextureViewDesc(gpuResourceView.GetViewType(), gpuResourceView.GetDesc()));
    }
}
