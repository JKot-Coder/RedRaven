#pragma once

#include "gapi/Device.hpp"
#include <thread>

namespace RR
{
    namespace GAPI::DX12
    {
        class FenceImpl;

        class DeviceImpl final : public IDevice
        {
        public:
            DeviceImpl();
            virtual ~DeviceImpl();

            bool Init(const IDevice::Description& description);
            void Present(const std::shared_ptr<SwapChain>& swapChain) override;
            void MoveToNextFrame(uint64_t frameIndex) override;

            GpuResourceFootprint GetResourceFootprint(const GpuResourceDescription& description) const override;

            void InitBuffer(const std::shared_ptr<Buffer>& resource) const override;
            void InitCommandList(CommandList& resource) const override;
            void InitCommandQueue(CommandQueue& resource) const override;
            void InitFence(Fence& resource) const override;
            void InitFramebuffer(Framebuffer& resource) const override;
            void InitGpuResourceView(GpuResourceView& view) const override;
            void InitSwapChain(SwapChain& resource) const override;
            void InitTexture(const std::shared_ptr<Texture>& resource) const override;

            std::any GetRawDevice() const override { return d3dDevice_.get(); }
            ID3D12Device* GetDevice() const { return d3dDevice_.get(); }

        private:
            void waitForGpu();
            bool createDevice();

        private:
            IDevice::Description description_ = {};

            std::atomic_bool inited_ = false;
            std::thread::id creationThreadID_;
            D3D_FEATURE_LEVEL d3dFeatureLevel_ = D3D_FEATURE_LEVEL_1_0_CORE;

            ComSharedPtr<IDXGIFactory2> dxgiFactory_;
            ComSharedPtr<IDXGIAdapter1> dxgiAdapter_;
            ComSharedPtr<ID3D12Device> d3dDevice_;
            std::shared_ptr<FenceImpl> gpuWaitFence_;
        };
    }
}