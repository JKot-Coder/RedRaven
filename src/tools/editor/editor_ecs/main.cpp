#include <imgui.h>
#include "cr.h"
#include "ecs_module\Context.hpp"
#include "ecs\Ecs.hpp"

struct TaskBar{};

struct  MySuperEvent : RR::Ecs::event
{
    MySuperEvent() : RR::Ecs::event(sizeof(MySuperEvent)) {};
    /* data */
};

#pragma clang optimize off
int init(const RR::EcsModule::Context& ctx)
{
    RR::Ecs::world& world = *ctx.editorWorld;
   // std::tuple<RR::Ecs::World> my_tuple = std::make_tuple(RR::Ecs::World{});

    UNUSED(world, ctx);
/*
    world.add<TaskBar>();
    ImGui::SetCurrentContext(ctx.imguiCtx);

world.system("TaskBar").with(EcsSystem).each([](flecs::entity e){
    Log::Format::Info("{}", e.name().c_str());
});
*/
    //SetAllocatorFunctions();
/*
    world.system<TaskBar>("TaskBar")
    .each([](const TaskBar&) {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File!"))
            {
                //ShowExampleMenuFile();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    });*/

    return 0;
}

CR_EXPORT int cr_main(struct cr_plugin *ctx, enum cr_op operation) {
    assert(ctx);
    assert(ctx->userdata);
    switch (operation) {
        case CR_LOAD:   return init(*(RR::EcsModule::Context*) ctx->userdata); // loading back from a reload
        case CR_UNLOAD: return 0; // preparing to a new reload
        case CR_CLOSE:  return 0; // the plugin will close and not reload anymore
        default: return 0; // CR_STEP
    }
}
#pragma clang optimize on