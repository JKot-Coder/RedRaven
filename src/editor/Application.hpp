#pragma once

 #include "platform/WindowSystem.hpp"

namespace RR
{
    class Application final : public Platform::Window::ICallbacks
    {
    public:
        int Run();

    private:
        void init();
    };
}