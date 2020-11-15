#include "gapi/DeviceInterface.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            class Device : public Render::Device
            {
            public:
                Device();
                ~Device() override;

                // Todo separate freeThread / Multithread
                Result Init() override;
                Result Reset(const PresentOptions& presentOptions) override;
                Result Present() override;

                Result Submit(const CommandContext::SharedPtr& commandContext) override;         

                uint64_t GetGpuFenceValue(const Fence::SharedPtr& fence) const override;

                Result InitResource(const Object::SharedPtr& resource) override;

                void WaitForGpu();

            private:
                std::unique_ptr<class DeviceImplementation> _impl;
            };
        }
    }
}