#include "ImGui.hpp"
#include "ecs/Ecs.hpp"
#include <imgui.h>

namespace RR
{
    namespace ImGuiEcs
    {
        void Init(RR::Ecs::World& world, ImGuiContext* ctx)
        {
            ImGui::SetCurrentContext(ctx);
            world.Entity().Add<Context>(Context{ctx}).Apply();
        }

        void Draw(RR::Ecs::World& world)
        {
            world.EmitImmediately<EarlyDrawEvent>({});
            world.EmitImmediately<DrawEvent>({});

            ImGui::Render();
        }
    }
}