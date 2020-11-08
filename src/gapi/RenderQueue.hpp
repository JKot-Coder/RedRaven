#pragma once

#include "gapi/Result.hpp"

#include "gapi/Object.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class CommandContextInterface;

        class RenderQueue final : public Object
        {
        public:
            RenderQueue() = delete;
            RenderQueue(const U8String& name)
                : Object(Object::Type::RenderQueue, name)
            {
            }

            virtual ~RenderQueue() = default;
        };
    }
}