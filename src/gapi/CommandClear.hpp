#pragma once

#include "common/Math.hpp"
#include "gapi/Command.hpp"
#include "gapi/ResourceViews.hpp"

namespace OpenDemo
{
    namespace Render
    {

        class CommandClearRenderTarget : public Command
        {
        public:
            CommandClearRenderTarget(RenderTargetView::ConstSharedPtrRef rtv, const Vector4& color)
                : Command(Type::CLEAR_RENDER_TARGET)
                //, rtv_(rtv)
                , color_(color)
            {
                std::ignore = rtv;
            }

        private:
            // RenderTargetView::SharedConstPtr rtv_;
            Vector4 color_;
        };

    }
}