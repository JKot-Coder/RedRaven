#pragma once

#include "windowing/Window.hpp"

#ifdef OS_WINDOWS
#include <Windows.h>
#endif

#include <GLFW/glfw3.h>

namespace OpenDemo
{
    namespace Windowing
    {
        class GlfwWindowImpl final : public IWindowImpl
        {
        public:
            ~GlfwWindowImpl();

            bool Init(Window::ICallbacks* callbacks, const Window::Description& description) override;

            void ShowCursor(bool value) override;
            int32_t GetWidth() const override;
            int32_t GetHeight() const override;
            std::any GetNativeHandle() const override;

        private:
            static void windowUpdateCallback(GLFWwindow* glfwWindow);
            static void windowResizeCallback(GLFWwindow* glfwWindow, int width, int height);
            static void windowCloseCallback(GLFWwindow* glfwWindow);

        private:
            Window::ICallbacks* callbacks_;
            GLFWwindow* window_;
#ifdef OS_WINDOWS
            HBRUSH bgBrush_;
#endif
        };
    }
}