#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/Event.hpp"

#include "EASTL/any.h"

struct GLFWwindow;
namespace RR::Ecs::WindowModule
{
    struct Tick : public RR::Ecs::Event
    {
        Tick() : Event(GetEventId<Tick>, sizeof(Tick)) { }
    };

    struct Window
    {
        struct Close : public Event
        {
            Close() : Event(GetEventId<Close>, sizeof(Close)) { }
        };

        struct Resize : public Event
        {
            Resize(int32_t width, int32_t height) : Event(GetEventId<Resize>, sizeof(Resize)), width(width), height(height) { }

            int32_t width;
            int32_t height;
        };

        GLFWwindow* glfwWindow = nullptr;
        eastl::any nativeHandle;
    };

    struct WindowDescription
    {
        int32_t width;
        int32_t height;
    };

    void Init(World& world);
}