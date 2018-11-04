#pragma once

#include <memory>
#include <vector>

namespace Windowing {

    struct WindowSettings;
    class Window;
    class IListener;

    class Windowing {
    public:
        Windowing();
        ~Windowing();

        static const Window& CreateWindow(const WindowSettings &settings);
        static void PoolEvents();
        static void Subscribe(const std::shared_ptr<IListener> &listener);
        static void UnSubscribe(const std::shared_ptr<IListener> &listener);
    private:
        static std::vector<std::shared_ptr<IListener>> listeners;
        static std::unique_ptr<Windowing> windowingInstance;
    };
}
