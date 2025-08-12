#pragma once

#include "ecs/Event.hpp"

struct ImGuiContext;

namespace RR::ImGuiEcs
{
    void Init(RR::Ecs::World& world, ImGuiContext* ctx);
    void Draw(RR::Ecs::World& world);

    struct Context
    {
        ECS_SINGLETON;
        ImGuiContext* ctx;
    };

    struct DrawEvent : Ecs::Event
    {
        DrawEvent() : Ecs::Event(Ecs::GetEventId<DrawEvent>, sizeof(DrawEvent)) { };
    };

    struct EarlyDrawEvent : Ecs::Event
    {
        EarlyDrawEvent() : Ecs::Event(Ecs::GetEventId<EarlyDrawEvent>, sizeof(EarlyDrawEvent)) { };
    };
}