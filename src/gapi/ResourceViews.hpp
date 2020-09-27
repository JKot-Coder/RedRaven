#pragma once

#include "gapi/Resource.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class RenderTargetView final : public Resource
        {
        public:
            RenderTargetView() = delete;
            RenderTargetView(const RenderTargetView&) = delete;
            RenderTargetView& operator=(const RenderTargetView&) = delete;
            RenderTargetView(const U8String& name)
                : Resource(Resource::Type::CommandList, name)
            {
            }

            virtual ~RenderTargetView() = default;
        };


    }
}