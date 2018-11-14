#pragma once

#include <memory>
#include <vector>

namespace Windowing {

    struct WindowSettings;
    class Window;
    class Listener;

    class Windowing {
    public:
        Windowing();
        ~Windowing();

        static const std::shared_ptr<Window> CreateWindow(const WindowSettings &settings);
        static void PoolEvents();
        static void Subscribe(Listener *listener);
        static void UnSubscribe(const Listener *listener);

    private:
        static std::vector<Listener*> listeners;
        static std::unique_ptr<Windowing> windowingInstance;
    };

}
