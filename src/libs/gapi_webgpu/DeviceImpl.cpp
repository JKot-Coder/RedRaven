#include "DeviceImpl.hpp"

#define ASSERT_IS_DEVICE_INITED ASSERT(inited)
#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

#include "Device.hpp"
#include "SwapChainImpl.hpp"
#include "TextureImpl.hpp"
#include "TextureViewImpl.hpp"

#include "gapi/Resource.hpp"
#include "gapi/Texture.hpp"

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

        auto resetOnExit = ([this]() {
            instance.release();
            device.release();
            inited = false;
        });

        wgpu::InstanceDescriptor instanceDesc;
        instanceDesc.setDefault();

        instance = wgpu::createInstance(instanceDesc);
        if(!instance)
        {
            Log::Format::Error("Failed to create WebGPU instance");
            resetOnExit();
            return false;
        }

        wgpu::RequestAdapterOptions requestAdapterOptions;
        requestAdapterOptions.setDefault();
        requestAdapterOptions.powerPreference = wgpu::PowerPreference::HighPerformance;

        wgpu::Adapter adapter = instance.requestAdapter(requestAdapterOptions);
        if(!adapter)
        {
            Log::Format::Error("Failed to request WebGPU adapter");
            resetOnExit();
            return false;
        }

        wgpu::DeviceDescriptor deviceDescriptor;
        deviceDescriptor.setDefault();
        deviceDescriptor.label = wgpu::StringView("Primary");

        device = adapter.requestDevice(deviceDescriptor);
        if(!device)
        {
            Log::Format::Error("Failed to request WebGPU device");
            resetOnExit();
            return false;
        }

        inited = true;
        return true;
    }

    void DeviceImpl::Present(SwapChain* swapChain)
    {
        ASSERT_IS_DEVICE_INITED;

        ASSERT(dynamic_cast<SwapChainImpl*>(swapChain->GetPrivateImpl()));
        auto swapChainImpl = static_cast<SwapChainImpl*>(swapChain->GetPrivateImpl());

        // WebGPU waits for gpu on Present()
        swapChainImpl->Present();
    }

    void DeviceImpl::MoveToNextFrame(uint64_t frameIndex)
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(frameIndex);
        // Nothing to do here. WebGPU handles frame waiting on swapChain->Present();
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

        const auto gpuResource = view.GetGpuResource().lock();
        if (gpuResource->GetDesc().IsTexture())
        {
            auto impl = std::make_unique<TextureViewImpl>();
            impl->Init(view);
            view.SetPrivateImpl(impl.release());
        }
        else
        {
            NOT_IMPLEMENTED();
        }
    }

    void DeviceImpl::InitSwapChain(SwapChain& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        auto impl = eastl::make_unique<SwapChainImpl>();
        impl->Init(instance, device, resource.GetDesc());
        resource.SetPrivateImpl(impl.release());
    }

    void DeviceImpl::InitBuffer(Buffer& resource, const BufferData* initialData) const
    {
        ASSERT_IS_DEVICE_INITED;

        UNUSED(resource, initialData);
        NOT_IMPLEMENTED();
    }

    void DeviceImpl::InitTexture(RR::GAPI::Texture& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        auto impl = std::make_unique<TextureImpl>();
        impl->Init(device, resource);
        resource.SetPrivateImpl(impl.release());
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