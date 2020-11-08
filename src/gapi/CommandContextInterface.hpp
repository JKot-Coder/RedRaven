#pragma once

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
        class RenderTargetView;

        class CommandContextInterface
        {
        public:
            virtual void Reset() = 0;

            virtual void ClearRenderTargetView(const RenderTargetView& renderTargetView, const Vector4& color) = 0;
        };
    }
}
