#pragma once

#include "gapi/CommandList2.hpp"

#include "webgpu/webgpu.hpp"

namespace RR::GAPI::WebGPU
{
    class CommandListImpl final : public ICommandList
    {
    public:
        CommandListImpl() = default;
        ~CommandListImpl();

        void Init(wgpu::Device device);
        void Compile();

    private:
        wgpu::CommandEncoder commandEncoder;
    };
}
