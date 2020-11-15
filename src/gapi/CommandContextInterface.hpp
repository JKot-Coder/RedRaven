#pragma once

#include "gapi/ResourceViews.hpp"

namespace OpenDemo
{
    namespace Common
    {
        template <size_t Len, typename T>
        struct Vector;

        using Vector4 = Vector<4, float>;
    }

    namespace Render
    {
        class CommandContextInterface
        {
        public:
            virtual void Reset() = 0;
            virtual void Close() = 0;

            virtual void ClearRenderTargetView(const RenderTargetView::SharedPtr& renderTargetView, const Vector4& color) = 0;
        };
    }
}
