#include "DeviceImpl.hpp"

#define ASSERT_IS_DEVICE_INITED ASSERT(inited)
#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

#include "Device.hpp"

#include "BindingLayoutImpl.hpp"
#include "BindingGroupImpl.hpp"
#include "BufferImpl.hpp"
#include "CommandListImpl.hpp"
#include "CommandQueueImpl.hpp"
#include "PipelineStateImpl.hpp"
#include "ShaderImpl.hpp"
#include "SwapChainImpl.hpp"
#include "TextureImpl.hpp"
#include "TextureViewImpl.hpp"

#include "gapi/BindingLayout.hpp"
#include "gapi/Buffer.hpp"
#include "gapi/PipelineState.hpp"
#include "gapi/Resource.hpp"
#include "gapi/Shader.hpp"
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

        wgpu::UncapturedErrorCallbackInfo uncapturedErrorCallbackInfo;
        uncapturedErrorCallbackInfo.setDefault();
        uncapturedErrorCallbackInfo.callback = [](WGPUDevice const* device, WGPUErrorType type, WGPUStringView message, void* userdata1, void* userdata2) {
            UNUSED(device, type, userdata1, userdata2);
            Log::Format::Error("Uncaptured error: {}", message.data);
        };

        wgpu::DeviceDescriptor deviceDescriptor;
        deviceDescriptor.setDefault();
        deviceDescriptor.label = wgpu::StringView("Primary");
        deviceDescriptor.uncapturedErrorCallbackInfo = uncapturedErrorCallbackInfo;

        device = adapter.requestDevice(deviceDescriptor);
        if(!device)
        {
            Log::Format::Error("Failed to request WebGPU device");
            resetOnExit();
            return false;
        }

        // Add an error callback for more debug info
        device.setLoggingCallback([](wgpu::LoggingType type, wgpu::StringView message) {
            UNUSED(type);
            Log::Format::Error("WebGPU: {}", message.data);
        });

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

    void DeviceImpl::Compile(CommandList& commandList)
    {
        ASSERT_IS_DEVICE_INITED;

        ASSERT(dynamic_cast<CommandListImpl*>(commandList.GetPrivateImpl()));
        auto commandListImpl = static_cast<CommandListImpl*>(commandList.GetPrivateImpl());

        commandListImpl->Compile(device, commandList);
    }

    void DeviceImpl::InitCommandList(CommandList& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        auto impl = eastl::make_unique<CommandListImpl>();
        impl->Init(device);
        resource.SetPrivateImpl(impl.release());
    }

    void DeviceImpl::InitCommandQueue(CommandQueue& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        if (queueInited)
        {
            Log::Format::Error("WebGPU: Only one command queue is allowed");
            return;
        }

        auto impl = eastl::make_unique<CommandQueueImpl>();
        impl->Init(device);
        resource.SetPrivateImpl(impl.release());
        queueInited = true;
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

        const auto gpuResource = view.GetGpuResource();
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

        auto impl = std::make_unique<BufferImpl>();
        impl->Init(device, resource, initialData);
        resource.SetPrivateImpl(impl.release());
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

        auto impl = std::make_unique<ShaderImpl>();
        impl->Init(device, resource);
        resource.SetPrivateImpl(impl.release());
    }

    void DeviceImpl::InitPipelineState(PipelineState& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        auto impl = eastl::make_unique<PipelineStateImpl>();
        impl->Init(device, resource);
        resource.SetPrivateImpl(impl.release());
    }

    void DeviceImpl::InitBindingLayout(BindingLayout& resource) const
    {
        ASSERT_IS_DEVICE_INITED;

        auto impl = eastl::make_unique<BindingLayoutImpl>();
        impl->Init(device, resource);
        resource.SetPrivateImpl(impl.release());
    }

    void DeviceImpl::InitBindingGroup(BindingGroup& resource, BindingGroupDesc& desc) const
    {
        ASSERT_IS_DEVICE_INITED;

        auto impl = eastl::make_unique<BindingGroupImpl>();
        impl->Init(device, resource, desc);
        resource.SetPrivateImpl(impl.release());
    }
}