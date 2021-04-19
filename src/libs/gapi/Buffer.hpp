#pragma once

#include "gapi/GpuResource.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        class Buffer final : public GpuResource
        {
        public:
            using SharedPtr = std::shared_ptr<Buffer>;
            using SharedConstPtr = std::shared_ptr<const Buffer>;

            static constexpr uint32_t MaxPossible = 0xFFFFFF;

        public:
            std::shared_ptr<ShaderResourceView> GetSRV(uint32_t firstElement = 0, uint32_t numElements = MaxPossible);
            std::shared_ptr<UnorderedAccessView> GetUAV(uint32_t firstElement = 0, uint32_t numElements = MaxPossible);

        private:
            template <class Deleter>
            static SharedPtr Create(
                const GpuResourceDescription& description,
                GpuResourceCpuAccess cpuAccess,
                const U8String& name,
                Deleter)
            {
                return SharedPtr(new Buffer(description, cpuAccess, name), Deleter());
            }

            Buffer(const GpuResourceDescription& description, GpuResourceCpuAccess cpuAccess, const U8String& name)
                : GpuResource(description, cpuAccess, name) {};

        private:
            friend class Render::DeviceContext;
        };
    }
}