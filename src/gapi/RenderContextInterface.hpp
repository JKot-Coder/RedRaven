#pragma once

#include "gapi/Object.hpp"

namespace OpenDemo
{
    namespace Render
    {
        class RenderTargetView;

        class CommandContextInterface
        {
        public:
            virtual void Reset() = 0;

            virtual void ClearRenderTargetView(const RenderTargetView& renderTargetView, const Vector4& color) = 0;
        };
    }
}

/*

   public:
            CommandContext() = delete;
            CommandContext(const CommandContext&) = delete;
            CommandContext& operator=(const CommandContext&) = delete;
            CommandContext(const U8String& name)
                : Resource(Resource::Type::CommandList, name)
            {
            }

            virtual ~CommandContext() = default;

        private:

*/