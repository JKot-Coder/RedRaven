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

        static const std::shared_ptr<Window> CreateWindow(const WindowSettings &settings);
        static void PoolEvents();
        static void Subscribe(IListener *listener);
        static void UnSubscribe(const IListener *listener);

    private:
        static std::vector<IListener*> listeners;
        static std::unique_ptr<Windowing> windowingInstance;
    };
}
