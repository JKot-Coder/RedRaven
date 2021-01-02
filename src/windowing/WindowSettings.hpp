#pragma once

namespace OpenDemo
{
    namespace Windowing
    {
        struct WindowRect
        {
            enum Position : int32_t
            {
                WINDOW_POSITION_UNDEFINED = -1,
                WINDOW_POSITION_CENTERED = -2
            };

            WindowRect()
                : X(0), Y(0), Width(0), Height(0) {};
            WindowRect(int32_t x, int32_t y, int32_t width, int32_t height)
                : X(x), Y(y), Width(width), Height(height) {};

            int32_t X, Y, Width, Height;
        };

        struct WindowSettings
        {
            WindowSettings()
                : Title(""), WindowRect() {};

            U8String Title;
            WindowRect WindowRect;
        };
    }
}