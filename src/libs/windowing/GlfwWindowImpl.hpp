#pragma once

#include "windowing/Window.hpp"

#ifdef OS_WINDOWS
#include <Windows.h>
#endif

#include <GLFW/glfw3.h>

namespace RR
{
    namespace Windowing
    {
        class GlfwWindowImpl final : public IWindowImpl
        {
        public:
            ~GlfwWindowImpl();

            bool Init(Window::ICallbacks* callbacks, const Window::Description& description) override;

            void ShowCursor(bool value) override;

            Vector2i GetSize() const override;
            void SetSize(const Vector2i& size) const override;

            Vector2i GetPosition() const override;
            void SetPosition(const Vector2i& position) const override;

            void SetTitle(const U8String& title) const override;
            void SetWindowAlpha(float alpha) const override;

            void Focus() const override;

            int32_t GetWindowAttribute(Window::Attribute attribute) const override;

            std::any GetNativeHandle() const override;
            std::any GetNativeHandle2() const override;

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