#pragma once

 #include "windowing/WindowSystem.hpp"

namespace RR
{
    class Application final : public Windowing::Window::ICallbacks
    {
    public:
        int Run();

    private:
        void init();
    };
}