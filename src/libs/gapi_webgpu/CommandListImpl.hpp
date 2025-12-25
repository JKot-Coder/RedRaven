#pragma once

#include "gapi/CommandList.hpp"

#include "webgpu/webgpu.hpp"

namespace RR::GAPI::WebGPU
{
    class CommandListImpl final : public ICommandList
    {
    public:
        CommandListImpl() = default;
        ~CommandListImpl();

        void Init(wgpu::Device device);
        void Compile(wgpu::Device device, GAPI::CommandList& commandList);

        wgpu::CommandBuffer TakeCommandBuffer()
        {
            wgpu::CommandBuffer tmp = nullptr;
            eastl::swap(commandBuffer, tmp);
            return tmp;
        }

    private:
        wgpu::CommandBuffer commandBuffer;
    };
}
