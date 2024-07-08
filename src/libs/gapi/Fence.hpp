#pragma once

#include "gapi/Resource.hpp"
#include <optional>

namespace RR
{
    namespace GAPI
    {
        static constexpr uint32_t INFINITY_WAIT = 0xFFFFFFFF;

        class IFence
        {
        public:
            virtual ~IFence() = default;

            virtual void Wait(std::optional<uint64_t> value, uint32_t timeout) const = 0;

            virtual uint64_t GetGpuValue() const = 0;
            virtual uint64_t GetCpuValue() const = 0;
        };

        class Fence final : public Resource<IFence>
        {
        public:
            using SharedPtr = std::shared_ptr<Fence>;
            using SharedConstPtr = std::shared_ptr<const Fence>;

            inline void Wait(std::optional<uint64_t> value, uint32_t timeout = GAPI::INFINITY_WAIT) const { return GetPrivateImpl()->Wait(value, timeout); }

            inline uint64_t GetGpuValue() const { return GetPrivateImpl()->GetGpuValue(); }
            inline uint64_t GetCpuValue() const { return GetPrivateImpl()->GetCpuValue(); }

        private:
            static SharedPtr Create(const std::string& name)
            {
                return SharedPtr(new Fence(name));
            }

            Fence(const std::string& name)
                : Resource(Object::Type::Fence, name)
            {
            }

        private:
            friend class Render::DeviceContext;
        };
    }
}