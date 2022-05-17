#include "Window.hpp"

#ifndef OS_WINDOWS
static_assert(false, "platform is not supported");
#endif

#ifdef OS_WINDOWS
#include "platform/GlfwWindowImpl.hpp"
#endif

namespace RR
{
    namespace Windowing
    {
        Window::~Window()
        {
            if (!impl_)
                return;

            impl_.reset();
        }

        bool Window::Init(Window::ICallbacks* callbacks, const Window::Description& description)
        {
            ASSERT(!impl_)

            impl_ = std::make_unique<GlfwWindowImpl>();
            return impl_->Init(callbacks, description);
        }
    }
}