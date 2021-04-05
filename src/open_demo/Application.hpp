#pragma once

#include "gapi/Device.hpp"
#include "windowing/WindowSystem.hpp"

namespace OpenDemo
{
    namespace Windowing
    {
        class InputtingWindow;
    }

    class Application : public Windowing::Window::ICallbacks
    {
    public:
        Application() = default;

        void Start();
        virtual void OnClose() override;

    private:
        bool _quit = false;

        std::shared_ptr<Windowing::Window> _window;
        std::shared_ptr<GAPI::SwapChain> swapChain_;

        void init();
        void terminate();

        void loadResouces();

        void OnWindowResize(uint32_t width, uint32_t height) override;
    };
}