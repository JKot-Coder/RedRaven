#pragma once

#include "platform/Window.hpp"

struct GLFWwindow;

namespace RR
{
    namespace Platform
    {
        class GlfwWindowImpl final : public Window
        {
        private:
            struct PlatformData;

        public:
            ~GlfwWindowImpl() override;

            Vector2i GetSize() const override;
            Vector2i GetFramebufferSize() const override;
            void SetSize(const Vector2i& size) const override;

            Vector2i GetPosition() const override;
            void SetPosition(const Vector2i& position) const override;

            Vector2i GetMousePosition() const override;
            void SetMousePosition(const Vector2i& position) const override;

            void SetTitle(const U8String& title) const override;
            void SetWindowAlpha(float alpha) const override;

            int32_t GetWindowAttribute(Window::Attribute attribute) const override;
            void SetWindowAttribute(Window::Attribute attribute, int32_t value) override;

            void SetClipboardText(const U8String& text) const override;
            U8String GetClipboardText() const override;

            void ShowCursor(bool value) override;
            void SetCursor(const std::shared_ptr<Cursor>& cursor) const override;

            std::any GetNativeHandle() const override;
            std::any GetNativeHandleRaw() const override;

            void Focus() const override;
            void Show() const override;

        public:
            static std::shared_ptr<Window> Create(const Window::Description& description);

        private:
            GlfwWindowImpl();

            static void charCallback(GLFWwindow* glfwWindow, unsigned int c);
            static void cursorEnterCallback(GLFWwindow* glfwWindow, int entered);
            static void cursorPosCallback(GLFWwindow* glfwWindow, double x, double y);
            static void keyCallback(GLFWwindow* glfwWindow, int keycode, int scancode, int action, int mods);
            static void mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods);
            static void scrollCallback(GLFWwindow* glfwWindow, double xoffset, double yoffset);
            static void windowCloseCallback(GLFWwindow* glfwWindow);
            static void windowContentScale(GLFWwindow* glfwWindow, float xscale, float yscale);
            static void windowFocusCallback(GLFWwindow* glfwWindow, int focused);
            static void windowPosCallback(GLFWwindow* glfwWindow, int x, int y);
            static void windowResizeCallback(GLFWwindow* glfwWindow, int width, int height);
            static void windowUpdateCallback(GLFWwindow* glfwWindow);

            void setWindowMousePassthrough(bool enabled);
            void setTaskbarIcon(bool enabled);

            bool init(const Window::Description& description);

        private:
            std::unique_ptr<PlatformData> platformData_;
            bool mousePassthrough_ = false;
            bool taskbarIcon_ = false;
            GLFWwindow* window_ = nullptr;
        };
    }
}