#include "SwapChainImpl.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

#include "gapi/GpuResource.hpp"
#include "gapi/Texture.hpp"

#include "TextureImpl.hpp"
#include "TextureViewImpl.hpp"

#if OS_WINDOWS
#include "windows.h"
#elif OS_APPLE
#include "SurfaceMetalLayer.h"
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
            case GAPI::GpuResourceFormat::BGRA8Unorm: return wgpu::TextureFormat::BGRA8Unorm;
              default:
                Log::Format::Error( "Invalid back buffer format: {}", GpuResourceFormatInfo::ToString(format));
                return wgpu::TextureFormat::Undefined;
        }
    }

    void SwapChainImpl::Init(const wgpu::Instance& instance, const wgpu::Device& device, const GAPI::SwapChainDesc& desc)
    {
        ASSERT(instance);
        ASSERT(device);

        wgpu::SurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.setDefault();
        surfaceDescriptor.label = wgpu::StringView("Surface");

#if OS_WINDOWS
        wgpu::SurfaceSourceWindowsHWND wgpuSurfaceNativeDesc;
        wgpuSurfaceNativeDesc.setDefault();
        wgpuSurfaceNativeDesc.hinstance = GetModuleHandle(nullptr);
        wgpuSurfaceNativeDesc.hwnd = eastl::any_cast<HWND>(desc.windowNativeHandle);
        surfaceDescriptor.nextInChain = &wgpuSurfaceNativeDesc.chain;
#elif OS_APPLE
        auto* nsWindow = eastl::any_cast<void*>(desc.windowNativeHandle);
        ASSERT(nsWindow != nullptr);
        auto* metalLayer = GetOrCreateMetalLayer(nsWindow);
        ASSERT(metalLayer != nullptr);

        wgpu::SurfaceSourceMetalLayer wgpuSurfaceNativeDesc;
        wgpuSurfaceNativeDesc.setDefault();
        wgpuSurfaceNativeDesc.layer = metalLayer;
        surfaceDescriptor.nextInChain = &wgpuSurfaceNativeDesc.chain;
#endif

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

        Resize(desc.width, desc.height);
    }

    void SwapChainImpl::Resize(uint32_t width, uint32_t height)
    {
        surfaceConfiguration.width = width;
        surfaceConfiguration.height = height;

        surface.configure(surfaceConfiguration);
        surface.getCurrentTexture(&surfaceTexture);
    }

    eastl::any SwapChainImpl::GetWaitableObject() const
    {
        NOT_IMPLEMENTED();
        return {};
    }

    void SwapChainImpl::UpdateBackBuffer(Texture& texture, RenderTargetView& rtv) const
    {
        // TODO Check resource description is valid
        if (!texture.GetPrivateImpl())
            texture.SetPrivateImpl(new TextureImpl());

        const auto textureImpl = texture.GetPrivateImpl<TextureImpl>();
        textureImpl->UpdateTextureResource(surfaceTexture);

        rtv.GetPrivateImpl<TextureViewImpl>()->RecreateView(rtv);
    }

    void SwapChainImpl::Present()
    {
        surface.present();
        surface.getCurrentTexture(&surfaceTexture);
    }
}
