#include "CommandQueueImpl.hpp"

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

#include "DeviceContext.h"
#include "CommandContextImpl.hpp"

namespace DL = ::Diligent;

namespace RR
{
    namespace GAPI::Diligent
    {
        CommandQueueImpl::~CommandQueueImpl() { }

        void CommandQueueImpl::Init(const DL::RefCntAutoPtr<DL::IDeviceContext>& deviceContext)
        {
            ASSERT(deviceContext);
            this->deviceContext = deviceContext;
        }

        std::any CommandQueueImpl::GetNativeHandle() const { NOT_IMPLEMENTED(); return nullptr; }
        void CommandQueueImpl::Signal(const eastl::shared_ptr<Fence>& fence) { NOT_IMPLEMENTED(); }
        void CommandQueueImpl::Signal(const eastl::shared_ptr<Fence>& fence, uint64_t value) { NOT_IMPLEMENTED(); }
        void CommandQueueImpl::Submit(const eastl::shared_ptr<CommandList>& commandList) { NOT_IMPLEMENTED(); }

        void CommandQueueImpl::WaitForGpu() { NOT_IMPLEMENTED(); }
    }
}
