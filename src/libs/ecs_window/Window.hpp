#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/Event.hpp"

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

        GLFWwindow* glfwWindow = nullptr;
    };

    void Init(World& world);
}