#pragma once

#include "gapi/Object.hpp"
#include "gapi/Result.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class CommandList;

        enum class CommandQueueType : uint32_t
        {
            Graphics,
            Compute,
            Copy,
            Count
        };

        class CommandQueueInterface
        {
        public:
            virtual Result Submit(const std::shared_ptr<CommandList>& commandList) = 0;
        };

        class CommandQueue final : public Object, public CommandQueueInterface
        {
        public:
            using SharedPtr = std::shared_ptr<CommandQueue>;
            using SharedConstPtr = std::shared_ptr<const CommandQueue>;

            CommandQueue() = delete;

            static SharedPtr Create(CommandQueueType type, const U8String& name)
            {
                return SharedPtr(new CommandQueue(type, name));
            }

            inline Result Submit(const std::shared_ptr<CommandList>& commandList) override { return getImplementation().Submit(commandList); }

            inline CommandQueueType GetType() const
            {
                return type_;
            }

        private:
            CommandQueue(CommandQueueType type, const U8String& name)
                : Object(Object::Type::CommandQueue, name),
                  type_(type)
            {
            }

            inline CommandQueueInterface& getImplementation()
            {
                ASSERT(privateImpl_);

                return *(static_cast<CommandQueueInterface*>(privateImpl_));
            }

            CommandQueueType type_;
        };
    }
}