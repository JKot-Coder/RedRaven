#pragma once

#include "gapi/GAPIResult.hpp"

#include "gapi/Resource.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class RenderContextInterface;

        class RenderQueue final : public Resource
        {
        public:
            RenderQueue() = delete;
            RenderQueue(const RenderQueue&) = delete;
            RenderQueue& operator=(const RenderQueue&) = delete;
            RenderQueue(const U8String& name)
                : Resource(Resource::Type::RenderQueue, name)
            {
            }

            virtual ~RenderQueue() = default;
        };
    }
}