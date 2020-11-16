#pragma once

#include "gapi/ForwardDeclarations.hpp"

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

            virtual void ClearRenderTargetView(const std::shared_ptr<RenderTargetView>& renderTargetView, const Vector4& color) = 0;
        };
    }
}
