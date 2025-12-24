#include "TextureImpl.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

#include "Utils.hpp"

namespace RR::GAPI::WebGPU
{
    TextureImpl::~TextureImpl() { }

    wgpu::TextureDimension getResourceDimension(const GpuResourceDimension& dimension)
    {
        switch (dimension)
        {
            case GpuResourceDimension::Texture1D: return wgpu::TextureDimension::_1D;
            case GpuResourceDimension::Texture2D: return wgpu::TextureDimension::_2D;
            case GpuResourceDimension::Texture3D: return wgpu::TextureDimension::_3D;
            default:
                ASSERT_MSG(false, "Unknown dimension");
                return wgpu::TextureDimension::Undefined;
        }
    }

    wgpu::TextureUsage getUsage(GpuResourceBindFlags bindFlags)
    {
        wgpu::TextureUsage flags = wgpu::TextureUsage::None;

        if (IsSet(bindFlags, GpuResourceBindFlags::ShaderResource))
            flags = flags | wgpu::TextureUsage::TextureBinding;
        if (IsSet(bindFlags, GpuResourceBindFlags::UnorderedAccess))
            flags = flags | wgpu::TextureUsage::StorageBinding;
        if (IsAny(bindFlags, GpuResourceBindFlags::RenderTarget | GpuResourceBindFlags::DepthStencil))
            flags = flags | wgpu::TextureUsage::CopySrc;

        static_assert(static_cast<uint32_t>(GpuResourceBindFlags::Count) == 4);
        return flags;
    }

    wgpu::TextureDescriptor getTextureDesc(const GpuResourceDesc& desc, const std::string& name)
    {
        ASSERT(desc.IsTexture());
        ASSERT(!IsSet(desc.bindFlags, GpuResourceBindFlags::IndexBuffer) && !IsSet(desc.bindFlags, GpuResourceBindFlags::VertexBuffer));

        wgpu::TextureDescriptor texDesc;
        texDesc.setDefault();
        texDesc.label = wgpu::StringView(name.c_str());
        texDesc.format = GetWGPUFormat(desc.texture.format);
        texDesc.dimension = getResourceDimension(desc.dimension);
        texDesc.size.width = desc.texture.width;
        texDesc.size.height = desc.texture.height;
        texDesc.size.depthOrArrayLayers = desc.texture.depth;
        texDesc.mipLevelCount = desc.texture.mipLevels;
        texDesc.sampleCount = desc.texture.arraySize;
        texDesc.usage = getUsage(desc.bindFlags);
        return texDesc;
    }

    void TextureImpl::Init(const wgpu::Device& device, const GpuResource& resource)
    {
        ASSERT_MSG(resource.GetDesc().IsTexture(), "Resource is not a texture");

        const auto desc = getTextureDesc(resource.GetDesc(), resource.GetName());
        texture = device.createTexture(desc);
    }

    void TextureImpl::UpdateTextureResource(const wgpu::SurfaceTexture& surfaceTexture)
    {
        // Just trust for caller code knows what he is doing
        texture = surfaceTexture.texture;
    }

    void TextureImpl::DestroyImmediatly() { NOT_IMPLEMENTED(); }

    std::any TextureImpl::GetRawHandle() const
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    std::vector<GpuResourceFootprint::SubresourceFootprint> TextureImpl::GetSubresourceFootprints(const GpuResourceDesc& desc) const
    {
        UNUSED(desc);
        NOT_IMPLEMENTED();
        return {};
    }

    wgpu::TextureView TextureImpl::CreateView(const wgpu::TextureViewDescriptor& desc) const
    {
        return texture.createView(desc);
    }

    void* TextureImpl::Map()
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    void TextureImpl::Unmap() { NOT_IMPLEMENTED(); }
}