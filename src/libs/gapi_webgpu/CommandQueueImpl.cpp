#include "CommandQueueImpl.hpp"

#include "CommandListImpl.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

namespace RR::GAPI::WebGPU
{
    CommandQueueImpl::~CommandQueueImpl() { }

    void CommandQueueImpl::Init(wgpu::Device device)
    {
        queue = device.getQueue();
    }

    std::any CommandQueueImpl::GetNativeHandle() const
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    void CommandQueueImpl::Signal(const eastl::shared_ptr<Fence>& fence)
    {
        UNUSED(fence);
        NOT_IMPLEMENTED();
    }

    void CommandQueueImpl::Signal(const eastl::shared_ptr<Fence>& fence, uint64_t value)
    {
        UNUSED(fence, value);
        NOT_IMPLEMENTED();
    }

    void CommandQueueImpl::Submit(const eastl::shared_ptr<CommandList>& commandList)
    {
        UNUSED(commandList);
        NOT_IMPLEMENTED();
    }

    void CommandQueueImpl::Submit(CommandList* commandList)
    {
        queue.submit(commandList->GetPrivateImpl<CommandListImpl>()->TakeCommandBuffer());
    }

    void CommandQueueImpl::WaitForGpu() { NOT_IMPLEMENTED(); }
}
