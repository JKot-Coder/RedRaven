#pragma once

#include "gapi/Resource.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        class IMemoryAllocation
        {
        public:
            virtual ~IMemoryAllocation() = default;
            virtual void* GetData() const = 0;
        };

        enum class MemoryAllocationType : uint32_t
        {
            Upload,
            Readback
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
            inline void* GetData() const { return GetPrivateImpl()->GetData(); }
            inline MemoryAllocationType GetMemoryType() const { return type_; }

        private:
            MemoryAllocationType type_;
            size_t size_;
        };
    }
}
