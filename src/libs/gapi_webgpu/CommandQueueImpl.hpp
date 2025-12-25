#pragma once

#include "gapi/ForwardDeclarations.hpp"
#include "gapi/CommandQueue.hpp"

#include "webgpu/webgpu.hpp"

namespace RR::GAPI::WebGPU
{
    class CommandQueueImpl final : public ICommandQueue
    {
    public:
        CommandQueueImpl() = default;
        ~CommandQueueImpl();

        void Init(wgpu::Device device);

        // TODO temporary
        std::any GetNativeHandle() const override;
        void Signal(const eastl::shared_ptr<Fence>& fence) override;
        void Signal(const eastl::shared_ptr<Fence>& fence, uint64_t value) override;
        void Submit(const eastl::shared_ptr<CommandList>& commandList) override;
        void Submit(CommandList2* commandList) override;
        void WaitForGpu() override;
     private:
        wgpu::Queue queue;
    };
}