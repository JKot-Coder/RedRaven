#include "WindowSystem.hpp"

#include "common/Exception.hpp"
#include "common/Math.hpp"

#include "Windowing/Window.hpp"

#include <GLFW/glfw3.h>

namespace OpenDemo
{
    namespace Windowing
    {
        WindowSystem::WindowSystem()
        {
        }

        WindowSystem::~WindowSystem()
        {
            glfwTerminate();
        }

        void WindowSystem::Init()
        {
            ASSERT(!isInited_);

            if (!glfwInit())
                LOG_FATAL("Can't initializate glfw");

            isInited_ = true;
        }

        std::shared_ptr<Window> WindowSystem::Create(Window::ICallbacks* callbacks, const Window::Description& description) const
        {
            ASSERT(isInited_);

            const auto& window = std::shared_ptr<Window>(new Window());

            if (!window->Init(callbacks, description))
                return nullptr;

            return window;
        }

        void WindowSystem::PoolEvents() const 
        {
            ASSERT(isInited_);

            glfwPollEvents();
        }
    }
}