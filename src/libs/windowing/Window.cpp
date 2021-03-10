#include "Window.hpp"

#include "windowing/Win32Window.hpp"

namespace OpenDemo
{
    namespace Windowing
    {
        Window::Window()
        {
        }

        Window::~Window()
        {
        }

        bool Window::Init(const WindowDescription& description)
        {
            ASSERT(!impl_);
            impl_ = std::make_unique<Win32Window>();
            return impl_->Init(description);
        }
    }
}