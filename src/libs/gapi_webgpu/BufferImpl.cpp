#include "BufferImpl.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

#include "Utils.hpp"

namespace RR::GAPI::WebGPU
{
    BufferImpl::~BufferImpl() { }

    wgpu::BufferUsage getBufferUsage(GpuResourceBindFlags bindFlags)
    {
        wgpu::BufferUsage flags = wgpu::BufferUsage::None;

        if (IsSet(bindFlags, GpuResourceBindFlags::IndexBuffer))
            flags = flags | wgpu::BufferUsage::Index;
        if (IsSet(bindFlags, GpuResourceBindFlags::VertexBuffer))
            flags = flags | wgpu::BufferUsage::Vertex;
        if (IsSet(bindFlags, GpuResourceBindFlags::ShaderResource))
            flags = flags | wgpu::BufferUsage::Storage;
        if (IsSet(bindFlags, GpuResourceBindFlags::UnorderedAccess))
            flags = flags | wgpu::BufferUsage::Uniform;

        // Buffers typically need copy operations
        flags = flags | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;

        return flags;
    }

    wgpu::BufferDescriptor getBufferDesc(const GpuResourceDesc& desc, const std::string& name)
    {
        ASSERT(desc.IsBuffer());
        ASSERT(!IsSet(desc.bindFlags, GpuResourceBindFlags::RenderTarget) && !IsSet(desc.bindFlags, GpuResourceBindFlags::DepthStencil));

        wgpu::BufferDescriptor bufDesc;
        bufDesc.setDefault();
        bufDesc.label = wgpu::StringView(name.c_str());
        bufDesc.size = desc.buffer.size;
        bufDesc.usage = getBufferUsage(desc.bindFlags);
        bufDesc.mappedAtCreation = false;
        return bufDesc;
    }

    void BufferImpl::Init(const wgpu::Device& device, const GpuResource& resource, const BufferData* initialData)
    {
        ASSERT_MSG(resource.GetDesc().IsBuffer(), "Resource is not a buffer");

        const auto desc = getBufferDesc(resource.GetDesc(), resource.GetName());
        buffer = device.createBuffer(desc);

        UNUSED(initialData);
        // TODO: Implement initial data
    }

    void BufferImpl::DestroyImmediatly() { NOT_IMPLEMENTED(); }

    std::any BufferImpl::GetRawHandle() const
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    std::vector<GpuResourceFootprint::SubresourceFootprint> BufferImpl::GetSubresourceFootprints(const GpuResourceDesc& desc) const
    {
        UNUSED(desc);
        NOT_IMPLEMENTED();
        return {};
    }

    void* BufferImpl::Map()
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    void BufferImpl::Unmap() { NOT_IMPLEMENTED(); }
}

