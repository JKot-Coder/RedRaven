#include "ecs/Ecs.hpp"
#include "ecs_imgui/ImGui.hpp"
#include "ecs_module/FeatureRegistry.hpp"
#include <imgui.h>

namespace RR::Editor
{
    void InitImguiContext(RR::Ecs::World& world)
    {
        world.System().OnEvent<Ecs::OnAppear>().With<ImGuiEcs::Context>().ForEach([](ImGuiEcs::Context context) {
            ImGui::SetCurrentContext(context.ctx);
        });
    }
    ECS_REGISTER_FEATURE(InitImguiContext);
}