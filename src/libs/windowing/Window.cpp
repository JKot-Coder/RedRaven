#include "Window.hpp"

#ifndef OS_WINDOWS
static_assert(false, "platform is not supported");
#endif

#ifdef OS_WINDOWS
#include "windowing/GlfwWindowImpl.hpp"
#endif

namespace OpenDemo
{
    namespace Windowing
    {
        Window::~Window()
        {
            if (!impl_)
                return;

            impl_.reset();
        }

        bool Window::Init(const WindowDescription& description)
        {
            ASSERT(!impl_)

            impl_ = std::make_unique<GlfwWindowImpl>();
            return impl_->Init(description);
        }
    }
}