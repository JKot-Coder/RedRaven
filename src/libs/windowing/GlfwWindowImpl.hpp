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
            ~GlfwWindowImpl() override;

            bool Init(Window::ICallbacks* callbacks, const Window::Description& description) override;

            void ShowCursor(bool value) override;

            Vector2i GetSize() const override;
            void SetSize(const Vector2i& size) const override;

            Vector2i GetPosition() const override;
            void SetPosition(const Vector2i& position) const override;

            void SetTitle(const U8String& title) const override;
            void SetWindowAlpha(float alpha) const override;

            int32_t GetWindowAttribute(Window::Attribute attribute) const override;
            void SetWindowAttribute(Window::Attribute attribute, int32_t value) override;

            std::any GetNativeHandle() const override;
            std::any GetNativeHandleRaw() const override;

            void Focus() const override;
            void Show() const override;

        private:
            static void windowUpdateCallback(GLFWwindow* glfwWindow);
            static void windowResizeCallback(GLFWwindow* glfwWindow, int width, int height);
            static void windowCloseCallback(GLFWwindow* glfwWindow);

            void setWindowMousePassthrough(bool enabled);
            void setTaskbarIcon(bool enabled);

        private:
            Window::ICallbacks* callbacks_;
            GLFWwindow* window_;
#ifdef OS_WINDOWS
            HBRUSH bgBrush_;
#endif
            bool mousePassthrough_ = false;
            bool taskbarIcon_ = false;
        };
    }
}