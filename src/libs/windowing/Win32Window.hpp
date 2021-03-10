#include "Window.hpp"

namespace OpenDemo
{
    namespace Windowing
    {
        class Win32Window final : public IWindowImpl
        {
            bool Init(const WindowDescription& description) override;

            int GetWidth() const override;
            int GetHeight() const override;

            void SetMousePos(int x, int y) const override;
            void ShowCursor(bool value) override;

            WindowHandle GetNativeHandle() const override;
        };
    }
}