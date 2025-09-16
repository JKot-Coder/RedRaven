#include "CommandQueueImpl.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

#include "DeviceContext.h"
#include "CommandListImpl.hpp"

namespace DL = ::Diligent;

namespace RR
{
    namespace GAPI::Diligent
    {
        CommandQueueImpl::~CommandQueueImpl() { }

        void CommandQueueImpl::Init(const DL::RefCntAutoPtr<DL::IDeviceContext>& deviceContext)
        {
            ASSERT(deviceContext);
            deviceContext_ = deviceContext;
        }

        std::any CommandQueueImpl::GetNativeHandle() const { NOT_IMPLEMENTED(); return nullptr; }
        void CommandQueueImpl::Signal(const eastl::shared_ptr<Fence>& fence) { UNUSED(fence); NOT_IMPLEMENTED(); }
        void CommandQueueImpl::Signal(const eastl::shared_ptr<Fence>& fence, uint64_t value) { UNUSED(fence, value); NOT_IMPLEMENTED(); }
        void CommandQueueImpl::Submit(const eastl::shared_ptr<CommandList>& commandList) { UNUSED(commandList); NOT_IMPLEMENTED(); }
        void CommandQueueImpl::Submit(CommandList2* commandList)
        {
            auto* commandContextImpl = static_cast<CommandListImpl*>(commandList->GetPrivateImpl());
            ASSERT(commandContextImpl);

            const eastl::array<DL::ICommandList*, 1> commandLists{ commandContextImpl->GetCommandList() };
            deviceContext_->ExecuteCommandLists(1, commandLists.data());

            commandContextImpl->Reset();
        }
        void CommandQueueImpl::WaitForGpu() { NOT_IMPLEMENTED(); }
    }
}
