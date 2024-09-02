#pragma once

#include "ecs\ForwardDeclarations.hpp"

struct ImGuiContext;

namespace RR::EcsModule
{
    struct Context final
    {
        Context(Ecs::world& editorWorld, ImGuiContext& imguiCtx) : editorWorld(&editorWorld), imguiCtx(&imguiCtx) { }
        Ecs::world* editorWorld;
        ImGuiContext* imguiCtx;
    };
}