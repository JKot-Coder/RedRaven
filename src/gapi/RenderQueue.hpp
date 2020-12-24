#pragma once

#include "gapi/Object.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class CommandQueue final : public Object
        {
        public:
            CommandQueue() = delete;
            CommandQueue(const U8String& name)
                : Object(Object::Type::CommandQueue, name)
            {
            }

            virtual ~CommandQueue() = default;
        };
    }
}