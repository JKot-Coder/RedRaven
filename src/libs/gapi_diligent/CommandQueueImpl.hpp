#pragma once

#include "gapi/ForwardDeclarations.hpp"

#include "gapi/CommandQueue.hpp"

#include "RefCntAutoPtr.hpp"

namespace Diligent
{
    struct IDeviceContext;
}

namespace DL = ::Diligent;

namespace RR
{
    namespace GAPI::Diligent
    {
        class CommandQueueImpl final : public ICommandQueue
        {
        public:
            CommandQueueImpl() = default;
            ~CommandQueueImpl();

            void Init(const DL::RefCntAutoPtr<DL::IDeviceContext>& deviceContext);

            // TODO temporary
            std::any GetNativeHandle() const override;
            void Signal(const eastl::shared_ptr<Fence>& fence) override;
            void Signal(const eastl::shared_ptr<Fence>& fence, uint64_t value) override;
            void Submit(const eastl::shared_ptr<CommandList>& commandList) override;
            void Submit(CommandList* commandList) override;
            void WaitForGpu() override;

        private:
            DL::RefCntAutoPtr<DL::IDeviceContext> deviceContext_;
        };
    }
}
