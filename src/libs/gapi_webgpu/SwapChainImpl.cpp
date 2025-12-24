#include "SwapChainImpl.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

#include "gapi/GpuResource.hpp"

#if OS_WINDOWS
#include "windows.h"
#endif

namespace RR::GAPI::WebGPU
{
    SwapChainImpl::~SwapChainImpl() { }

    wgpu::PresentMode getPresentMode(GAPI::SwapChainDesc::PresentMode presentMode)
    {
        switch(presentMode)
        {
            case GAPI::SwapChainDesc::PresentMode::Fifo: return wgpu::PresentMode::Fifo;
            case GAPI::SwapChainDesc::PresentMode::Mailbox: return wgpu::PresentMode::Mailbox;
            case GAPI::SwapChainDesc::PresentMode::Immediate: return wgpu::PresentMode::Immediate;
            default:
                ASSERT_MSG(false, "Invalid present mode");
                return wgpu::PresentMode::Fifo;
        }
    }

    wgpu::TextureFormat getWGPUFormat(GAPI::GpuResourceFormat format)
    {
        if(GpuResourceFormatInfo::IsSRGB(format))
        {
            Log::Format::Error("{} is SRGB format they are not supported in WebGPU, use regular format and SRGB RTV instead", GpuResourceFormatInfo::ToString(format));
            return wgpu::TextureFormat::Undefined;
        }

        switch(format)
        {
            case GAPI::GpuResourceFormat::RGBA8Unorm: return wgpu::TextureFormat::RGBA8Unorm;
            default:
                Log::Format::Error( "Invalid back buffer format: {}", GpuResourceFormatInfo::ToString(format));
                return wgpu::TextureFormat::Undefined;
        }
    }

    void SwapChainImpl::Init(const wgpu::Instance& instance, const wgpu::Device& device, const GAPI::SwapChainDesc& desc)
    {
        ASSERT(instance);
        ASSERT(device);

#if OS_WINDOWS
        wgpu::SurfaceSourceWindowsHWND wgpuSurfaceNativeDesc;
        wgpuSurfaceNativeDesc.setDefault();
        wgpuSurfaceNativeDesc.hinstance = GetModuleHandle(nullptr);
        wgpuSurfaceNativeDesc.hwnd = eastl::any_cast<HWND>(desc.windowNativeHandle);
#endif

        wgpu::SurfaceDescriptor surfaceDescriptor = {};
        surfaceDescriptor.nextInChain = &wgpuSurfaceNativeDesc.chain;
        surfaceDescriptor.label = wgpu::StringView("Surface");

        surface = instance.createSurface(surfaceDescriptor);
        if(!surface)
        {
            Log::Format::Error("Failed to create WebGPU surface");
            return;
        }

        surfaceConfiguration.setDefault();
        surfaceConfiguration.device = device;
        surfaceConfiguration.format = getWGPUFormat(desc.backBufferFormat);
        surfaceConfiguration.presentMode = getPresentMode(desc.presentMode);
        surfaceConfiguration.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst;

        Resize(desc.width, desc.height, {});
    }

    void SwapChainImpl::Resize(uint32_t width, uint32_t height, const eastl::array<GAPI::Texture*, MAX_BACK_BUFFERS_COUNT>& backBuffers)
    {
        UNUSED(backBuffers);

        surfaceConfiguration.width = width;
        surfaceConfiguration.height = height;

        surface.configure(surfaceConfiguration);
    }

    eastl::any SwapChainImpl::GetWaitableObject() const
    {
        NOT_IMPLEMENTED();
        return {};
    }

    uint32_t SwapChainImpl::GetCurrentBackBufferIndex() const
    {
        return 0;
    }

    void SwapChainImpl::InitBackBufferTexture(uint32_t backBufferIndex, Texture& resource) const
    {
        ASSERT(backBufferIndex == 0);
        UNUSED(backBufferIndex, resource);

      //  surface.getCurrentTexture(&surfaceTexture);
    }

    void SwapChainImpl::Present()
    {
        surface.present();
    }
}