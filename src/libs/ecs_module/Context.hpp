#pragma once

struct ImGuiContext;

namespace RR::Ecs
{
    class World;
}

namespace RR::EcsModule
{
    struct Context final
    {
        Context(Ecs::World& editorWorld, ImGuiContext& imguiCtx) : editorWorld(&editorWorld), imguiCtx(&imguiCtx) { }
        Ecs::World* editorWorld;
        ImGuiContext* imguiCtx;
    };
}