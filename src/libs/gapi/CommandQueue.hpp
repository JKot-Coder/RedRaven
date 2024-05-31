#pragma once

#include "gapi/Resource.hpp"
// TODO temporary
#include <any>

namespace RR
{
    namespace Render
    {
        class DeviceContext;
    }

    namespace GAPI
    {
        class CommandList;

        enum class CommandQueueType : uint32_t
        {
            Graphics,
            Compute,
            Copy,
            Count
        };

        class ICommandQueue
        {
        public:
            virtual ~ICommandQueue() = default;

            // TODO temporary
            virtual std::any GetNativeHandle() const = 0;
            virtual void Signal(const std::shared_ptr<Fence>& fence) = 0;
            virtual void Signal(const std::shared_ptr<Fence>& fence, uint64_t value) = 0;
            virtual void Submit(const std::shared_ptr<CommandList>& commandList) = 0;
            virtual void WaitForGpu() = 0;
        };

        class CommandQueue final : public Resource<ICommandQueue>
        {
        public:
            using SharedPtr = std::shared_ptr<CommandQueue>;
            using SharedConstPtr = std::shared_ptr<const CommandQueue>;

            CommandQueue() = delete;

            inline std::any GetNativeHandle() const { return GetPrivateImpl()->GetNativeHandle(); }
            inline void Signal(const std::shared_ptr<Fence>& fence) { return GetPrivateImpl()->Signal(fence); }
            inline void Signal(const std::shared_ptr<Fence>& fence, uint64_t value) { return GetPrivateImpl()->Signal(fence, value); }
            inline void Submit(const std::shared_ptr<CommandList>& commandList) { return GetPrivateImpl()->Submit(commandList); }

            inline CommandQueueType GetCommandQueueType() const { return type_; }

        private:
            static SharedPtr Create(CommandQueueType type, const U8String& name)
            {
                return SharedPtr(new CommandQueue(type, name));
            }

            CommandQueue(CommandQueueType type, const U8String& name)
                : Resource(Object::Type::CommandQueue, name),
                  type_(type)
            {
            }

            inline void WaitForGpu() { GetPrivateImpl()->WaitForGpu(); }

        private:
            CommandQueueType type_;

            friend class Render::DeviceContext;
        };
    }
}