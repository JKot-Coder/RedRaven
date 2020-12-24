#pragma once

#include "gapi/Object.hpp"

namespace OpenDemo
{
    namespace Render
    {
        enum class CommandQueueType : uint32_t
        {
            Graphics,
            Compute,
            Copy,
            Count
        };

        class CommandQueue final : public Object
        {
        public:
            using SharedPtr = std::shared_ptr<CommandQueue>;
            using SharedConstPtr = std::shared_ptr<const CommandQueue>;

            CommandQueue() = delete;

            static SharedPtr Create(CommandQueueType type, const U8String& name)
            {
                return SharedPtr(new CommandQueue(type, name));
            }

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

            CommandQueueType type_;
        };
    }
}