#pragma once

#include "gapi/Device.hpp"
#include "platform/Toolkit.hpp"

namespace RR
{
    namespace Platform
    {
        class Window;
    }

    class Application
    {
    public:
        Application() = default;

        void Start();
        void OnClose(const Platform::Window&);

    private:
        bool _quit = false;

        std::shared_ptr<Platform::Window> _window;
        std::shared_ptr<GAPI::SwapChain> swapChain_;

        void init();
        void terminate();

        void loadResouces();

        void OnWindowResize(const Platform::Window&, const Vector2i& size);
    };
}