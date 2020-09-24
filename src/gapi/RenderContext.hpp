#pragma once

#include "gapi/Resource.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class RenderContext final : public Resource
        {
        public:
            RenderContext() = delete;
            RenderContext(const RenderContext&) = delete;
            RenderContext& operator=(const RenderContext&) = delete;
            RenderContext(const U8String& name)
                : Resource(Resource::Type::CommandList, name)
            {
            }

            virtual ~RenderContext() = default;

        private:
        };
    }
}