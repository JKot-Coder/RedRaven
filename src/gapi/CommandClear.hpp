#pragma once

#include "gapi/Command.hpp"
#include "gapi/ResourceViews.hpp"

namespace OpenDemo
{
    namespace Render
    {

        class CommandClearRenderTarget : public Command
        {
        public:
            CommandClearRenderTarget(RenderTargetView::SharedConstPtr rtv)
                : Command(Type::CLEAR_RENDER_TARGET)
            {
            }
        };

    }
}