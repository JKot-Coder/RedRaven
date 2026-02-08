#pragma once

#include "gapi/GpuResource.hpp"

#include "webgpu/webgpu.hpp"

namespace RR::GAPI::WebGPU
{
    class BufferImpl final : public RR::GAPI::IGpuResource
    {
    public:
        BufferImpl() = default;
        ~BufferImpl() override;

        void Init(const wgpu::Device& device, const GpuResource& resource, const BufferData* initialData);

        void DestroyImmediatly() override;
        std::any GetRawHandle() const override;
        std::vector<GpuResourceFootprint::SubresourceFootprint> GetSubresourceFootprints(const GpuResourceDesc& desc) const override;

        void* Map() override;
        void Unmap() override;

        wgpu::Buffer GetBuffer() const { return buffer; }
        wgpu::IndexFormat GetIndexFormat() const { return indexFormat; }
        size_t GetSize() const { return size; }

    private:
        wgpu::Buffer buffer;
        wgpu::IndexFormat indexFormat;
        size_t size;
    };
}

