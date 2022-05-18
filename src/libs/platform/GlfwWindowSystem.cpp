#include "WindowSystem.hpp"

#include "platform/Window.hpp"
#include "platform/GlfwWindowImpl.hpp"

namespace RR
{
    namespace Platform
    {

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

            const auto& window = std::shared_ptr<Window>(new GlfwWindowImpl());

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