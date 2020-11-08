#pragma once

#include "gapi/Object.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class RenderTargetView;

        class RenderCommandContextInterface
        {
        public:
            virtual void Reset() = 0;

            virtual void ClearRenderTargetView(const RenderTargetView& renderTargetView, const Vector4& color) = 0;
        };
    }
}

/*

   public:
            RenderCommandContext() = delete;
            RenderCommandContext(const RenderCommandContext&) = delete;
            RenderCommandContext& operator=(const RenderCommandContext&) = delete;
            RenderCommandContext(const U8String& name)
                : Resource(Resource::Type::CommandList, name)
            {
            }

            virtual ~RenderCommandContext() = default;

        private:

*/