#pragma once

#include "gapi/Resource.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class RenderTargetView;

        class RenderContextInterface
        {
        public:
            virtual void Reset() = 0;

            virtual void ClearRenderTargetView(const RenderTargetView& renderTargetView, const Vector4& color) = 0;
        };
    }
}

/*

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

*/