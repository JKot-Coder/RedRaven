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

        class MemoryAllocation final : public Resource<IMemoryAllocation, false>
        {
        public:
            using SharedPtr = std::shared_ptr<MemoryAllocation>;
            using SharedConstPtr = std::shared_ptr<const MemoryAllocation>;

            enum class Type
            {
                UploadBuffer
            };

            MemoryAllocation(Type memoryType) : type(memoryType), Resource(Object::Type::MemoryAllocation) { }

            inline void* GetData() const { return GetPrivateImpl()->GetData(); }
            inline Type GetMemoryType() const { return type; }

        private:
            Type type;
        };
    }
}
