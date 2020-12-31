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
            virtual Result Signal(const std::shared_ptr<CommandQueue>& queue) = 0;

            virtual Result SyncCPU(std::optional<uint64_t> value, uint32_t timeout) const = 0;
            virtual Result SyncGPU(const std::shared_ptr<CommandQueue>& queue) const = 0;

            virtual uint64_t GetGpuValue() const = 0;
            virtual uint64_t GetCpuValue() const = 0;
        };

        class Fence final : public InterfaceWrapObject<FenceInterface>
        {
        public:
            using SharedPtr = std::shared_ptr<Fence>;
            using SharedConstPtr = std::shared_ptr<const Fence>;

            inline Result Signal(const std::shared_ptr<CommandQueue>& queue) { return GetInterface()->Signal(queue); }
            inline Result SyncCPU(std::optional<uint64_t> value, uint32_t timeout) const { return GetInterface()->SyncCPU(value, timeout); }
            inline Result SyncGPU(const std::shared_ptr<CommandQueue>& queue) const { return GetInterface()->SyncGPU(queue); }

            inline uint64_t GetGpuValue() const { return GetInterface()->GetGpuValue(); }
            inline uint64_t GetCpuValue() const { return GetInterface()->GetCpuValue(); }

        private:
            static SharedPtr Create(const U8String& name)
            {
                return SharedPtr(new Fence(name));
            }

            Fence(const U8String& name)
                : InterfaceWrapObject(Object::Type::Fence, name)
            {
            }

        private:
            friend class Render::RenderContext;
        };

    }
}