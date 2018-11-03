#pragma once

namespace Windowing{

    struct WindowSettings;

    class Window
    {
    public:
        void Init(const WindowSettings &settings);
        bool IsWindow() const;
    };

}