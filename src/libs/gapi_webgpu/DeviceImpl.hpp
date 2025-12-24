#pragma once

#include "gapi/Device.hpp"
#include "webgpu/webgpu.hpp"

namespace RR::GAPI::WebGPU
{
    class DeviceImpl final : public IDevice
    {
    public:
        DeviceImpl();
        ~DeviceImpl() override;

        bool Init(const GAPI::DeviceDesc& desc);
        void Present(SwapChain* swapChain) override;
        void MoveToNextFrame(uint64_t frameIndex) override;

        GpuResourceFootprint GetResourceFootprint(const GpuResourceDesc& desc) const override;

        void Compile(CommandList2& commandList) override;

        void InitCommandList(CommandList& resource) const override;
        void InitCommandList2(CommandList2& resource) const override;
        void InitCommandQueue(CommandQueue& resource) const override;
        void InitFence(Fence& resource) const override;
        void InitGpuResourceView(GpuResourceView& view) const override;
        void InitSwapChain(SwapChain& resource) const override;
        void InitBuffer(Buffer& resource, const BufferData* initialData) const override;
        void InitTexture(Texture& resource) const override;
        void InitShader(Shader& resource) const override;
        void InitPipelineState(PipelineState& resource) const override;

        std::any GetRawDevice() const override { ASSERT_MSG(false, "Not implemented"); return nullptr; }

    private:
        bool inited = false;
        mutable bool queueInited = false;
        wgpu::Instance instance;
        wgpu::Device device;
        GAPI::DeviceDesc desc = {};
    };
}