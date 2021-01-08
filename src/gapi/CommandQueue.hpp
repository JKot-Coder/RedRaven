#pragma once

#include "gapi/Resource.hpp"
#include "gapi/Result.hpp"

namespace OpenDemo
{
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
            virtual ~ICommandQueue() {};

            virtual Result Submit(const std::shared_ptr<CommandList>& commandList) = 0;
        };

        class CommandQueue final : public Resource<ICommandQueue>
        {
        public:
            using SharedPtr = std::shared_ptr<CommandQueue>;
            using SharedConstPtr = std::shared_ptr<const CommandQueue>;

            CommandQueue() = delete;

            static SharedPtr Create(CommandQueueType type, const U8String& name)
            {
                return SharedPtr(new CommandQueue(type, name));
            }

            inline Result Submit(const std::shared_ptr<CommandList>& commandList) { return GetPrivateImpl()->Submit(commandList); }

            inline CommandQueueType GetType() const
            {
                return type_;
            }

        private:
            CommandQueue(CommandQueueType type, const U8String& name)
                : Resource(Object::Type::CommandQueue, name),
                  type_(type)
            {
            }

            CommandQueueType type_;
        };
    }
}