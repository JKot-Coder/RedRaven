#pragma once

#include "ecs\ForwardDeclarations.hpp"

struct ImGuiContext;

namespace RR::EcsModule
{
    struct Context final
    {
        Context(Ecs::World& editorWorld, ImGuiContext& imguiCtx) : editorWorld(&editorWorld), imguiCtx(&imguiCtx) { }
        Ecs::World* editorWorld;
        ImGuiContext* imguiCtx;
    };
}