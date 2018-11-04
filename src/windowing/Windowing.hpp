#pragma once

#include <memory>

namespace Windowing {

    struct WindowSettings;
    class Window;

    class Windowing {
    public:
        Windowing();
        ~Windowing();

        static const Window& CreateWindow(const WindowSettings &settings);
        static void PoolEvents();
    private:
        static std::unique_ptr<Windowing> windowingInstance;
    };
}
