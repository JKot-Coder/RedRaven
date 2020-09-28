#pragma once

#include "gapi/Result.hpp"

#include "gapi/Object.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class RenderContextInterface;

        class RenderQueue final : public Object
        {
        public:
            RenderQueue() = delete;
            RenderQueue(const RenderQueue&) = delete;
            RenderQueue& operator=(const RenderQueue&) = delete;
            RenderQueue(const U8String& name)
                : Object(Object::Type::RenderQueue, name)
            {
            }

            virtual ~RenderQueue() = default;
        };
    }
}