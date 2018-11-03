#pragma once

#include <memory>

namespace Windowing {

    struct WindowSettings;
    class Window;

    class Windowing {
    public:
        static const Window& CreateWindow(const WindowSettings &settings);
    private:
        static std::unique_ptr<Windowing> windowingInstance;
    };
}
