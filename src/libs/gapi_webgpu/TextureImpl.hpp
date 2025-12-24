#pragma once

#include "gapi/GpuResource.hpp"

#include "webgpu/webgpu.hpp"

namespace RR::GAPI::WebGPU
{
    class TextureImpl final : public RR::GAPI::IGpuResource
    {
    public:
        TextureImpl() = default;
        ~TextureImpl() override;

        void Init(const wgpu::Device& device, const GpuResource& resource);
        void UpdateTextureResource(const wgpu::SurfaceTexture& surfaceTexture);

        void DestroyImmediatly() override;
        std::any GetRawHandle() const override;
        std::vector<GpuResourceFootprint::SubresourceFootprint> GetSubresourceFootprints(const GpuResourceDesc& desc) const override;
        wgpu::TextureView CreateView(const wgpu::TextureViewDescriptor& desc) const;

        void* Map() override;
        void Unmap() override;

    private:
        wgpu::Texture texture;
    };
}