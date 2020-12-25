#pragma once

#include "gapi/Object.hpp"
#include <optional>

namespace OpenDemo
{
    namespace GAPI
    {

        class FenceInterface
        {
        public:
            virtual Result SyncCPU(std::optional<uint64_t> value, uint32_t timeout) const = 0;
            virtual Result SyncGPU(const std::shared_ptr<CommandQueue>& queue) const = 0;

            virtual uint64_t GetGpuValue() const = 0;
            virtual uint64_t GetCpuValue() const = 0;
        };

        class Fence final : public Object, public FenceInterface
        {
        public:
            using SharedPtr = std::shared_ptr<Fence>;
            using SharedConstPtr = std::shared_ptr<const Fence>;

            inline Result SyncCPU(std::optional<uint64_t> value, uint32_t timeout) const override
            {
                return getImplementation().SyncCPU(value, timeout);
            }
            
            inline Result SyncGPU(const std::shared_ptr<CommandQueue>& queue) const override
            {
                return getImplementation().SyncGPU(queue);
            }

            inline uint64_t GetGpuValue() const override { return getImplementation().GetGpuValue(); }
            inline uint64_t GetCpuValue() const override { return getImplementation().GetCpuValue(); }

        private:
            static SharedPtr Create(const U8String& name)
            {
                return SharedPtr(new Fence(name));
            }

            Fence(const U8String& name)
                : Object(Object::Type::Fence, name)
            {
            }

            inline FenceInterface& getImplementation() const
            {
                ASSERT(privateImpl_);

                return *(static_cast<FenceInterface*>(privateImpl_));
            }

        private:
            friend class Render::RenderContext;
        };

    }
}