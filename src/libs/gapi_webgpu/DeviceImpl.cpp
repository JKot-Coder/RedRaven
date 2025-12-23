#include "DeviceImpl.hpp"

#define ASSERT_IS_DEVICE_INITED ASSERT(inited)
#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

#include "Device.hpp"

#define WEBGPU_CPP_IMPLEMENTATION
#include "webgpu/webgpu.hpp"

namespace RR::GAPI::WebGPU
{
    DeviceImpl::DeviceImpl() { }
    DeviceImpl::~DeviceImpl()
    {
        if (!inited)
            return;
    }

    bool DeviceImpl::Init(const DeviceDesc& deviceDesc)
    {
        ASSERT(!inited);
        UNUSED(deviceDesc);

        const wgpu::InstanceDescriptor instanceDescriptor = {};
        wgpu::Instance instance = wgpu::createInstance(instanceDescriptor);

        if(!instance)
        {
            Log::Format::Error("Failed to create WebGPU instance");
            return false;
        }

        wgpu::RequestAdapterOptions requestAdapterOptions = {};
        requestAdapterOptions.powerPreference = wgpu::PowerPreference::HighPerformance;

        wgpu::Adapter adapter = instance.requestAdapter(requestAdapterOptions);
        if(!adapter)
        {
            Log::Format::Error("Failed to request WebGPU adapter");
            return false;
        }

        wgpu::DeviceDescriptor deviceDescriptor = {};
        deviceDescriptor.label = wgpu::StringView("Primary");

        wgpu::Device device = adapter.requestDevice(deviceDescriptor);
        if(!adapter)
        {
            Log::Format::Error("Failed to request WebGPU device");
            return false;
        }

        UNUSED(device);
        return true;
    }

    void DeviceImpl::Present(SwapChain* swapChain)
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(swapChain);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::MoveToNextFrame(uint64_t frameIndex)
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(frameIndex);
        NOT_IMPLEMENTED();
    }

    GpuResourceFootprint DeviceImpl::GetResourceFootprint(const GpuResourceDesc& resourceDesc) const
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(resourceDesc);
        NOT_IMPLEMENTED();
        return {};
    }

    void DeviceImpl::Compile(CommandList2& commandList)
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(commandList);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitCommandList(CommandList& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitCommandList2(CommandList2& resource) const
    {
        ASSERT_IS_DEVICE_INITED;
        UNUSED(resource);

        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitCommandQueue(CommandQueue& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitFence(Fence& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitGpuResourceView(GpuResourceView& view) const
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(view);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitSwapChain(SwapChain& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitBuffer(Buffer& resource, const BufferData* initialData) const
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(resource, initialData);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitTexture(Texture& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitShader(Shader& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(resource);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitPipelineState(PipelineState& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(resource);
        NOT_IMPLEMENTED();
    }
}