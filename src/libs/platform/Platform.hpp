#pragma once

#include "common/Math.hpp"

namespace RR::Platform
{
    struct VideoMode
    {
        Vector2i resolution;
        uint32_t redBits;
        uint32_t greenBits;
        uint32_t blueBits;
        uint32_t refreshRate;
    };

    struct Monitor
    {
        Vector2i position;
        VideoMode videoMode;
        Rect2i workArea;
        Vector2 dpiScale;
    };

    std::vector<Monitor> GetMonitors();
}