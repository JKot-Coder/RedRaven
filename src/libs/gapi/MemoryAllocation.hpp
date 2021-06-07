#pragma once

#include "gapi/Resource.hpp"

namespace RR
{
    namespace GAPI
    {
        class IMemoryAllocation
        {
        public:
            virtual ~IMemoryAllocation() = default;
            virtual void* Map() const = 0;
            virtual void Unmap() const = 0;
        };

        enum class MemoryAllocationType : uint32_t
        {
            CpuReadWrite,
            Upload,
            Readback,
            Count
        };

        class MemoryAllocation final : public Resource<IMemoryAllocation, false>
        {
        public:
            using SharedPtr = std::shared_ptr<MemoryAllocation>;
            using SharedConstPtr = std::shared_ptr<const MemoryAllocation>;

            MemoryAllocation(MemoryAllocationType memoryType, size_t size) : type_(memoryType),
                                                                             size_(size), Resource(Object::Type::MemoryAllocation)
            {
            }

            inline size_t GetSize() const { return size_; }
            inline void* Map() const { return GetPrivateImpl()->Map(); }
            inline void Unmap() const { GetPrivateImpl()->Unmap(); }
            inline MemoryAllocationType GetMemoryType() const { return type_; }

        private:
            MemoryAllocationType type_;
            size_t size_;
        };
    }
}
