#include "cr.h"
#include "ecs\Ecs.hpp"
#include "ecs_module\Context.hpp"
#include <imgui.h>

struct TaskBar
{
};

struct MySuperEvent : RR::Ecs::Event
{
    MySuperEvent() : RR::Ecs::Event(sizeof(MySuperEvent)) { };
    /* data */
};

struct Position
{
    float x, y;
};


struct Velocity
{
    float x;
};

struct MyEvent
{
};

struct BlaBla
{
    float g;
};

struct BlaBla2
{
    float g;
};

void qwe_ES(Position&)
{
    RR::Common::Debug::Log::Format::Info("!!!!1111!");
}


#pragma clang optimize off
int init(const RR::EcsModule::Context& ctx)
{
    RR::Ecs::World& world = *ctx.editorWorld;
    std::ignore = world;

    std::array<int, 2> z;
    const auto s = world.System<Position>("asd").OnEvent<MySuperEvent>([z](const MySuperEvent&) {
        RR::Common::Debug::Log::Format::Info("!!!!!", z[0]);
    });

    const auto s1 = world.System<Position>("asd").OnEvent<MySuperEvent>([z](const RR::Ecs::Event&) {
        RR::Common::Debug::Log::Format::Info("!!!!!", z[0]);
    });

    const auto s2 = world.System<Position>("asd").OnEvent<MySuperEvent>([z]() {
        RR::Common::Debug::Log::Format::Info("!!!!!", z[0]);
    });

  auto args = std::make_tuple(42, 3.14f);

    MyType* ptr = reinterpret_cast<MyType*>(storage);
    std::apply([ptr](auto&&... args) {
        new (ptr) MyType(std::forward<decltype(args)>(args)...);
    }, args);

   // auto entt1 = world.Entity().Add<Position>(1.0f,1.0f).Commit();
    world.Entity(entt1).Add<Velocity>(1.0f).Commit();
    world.Entity(entt1).Add<BlaBla>(1.0f).Commit();
    world.Entity().Add<Position>(1.0f,1.0f).Commit();
     world.Entity().Add<Position>(1.0f,1.0f).Commit();
 /*    world.Entity().Add<Position>(2.0f,1.0f).Commit();
      world.Entity().Add<Position>(3.0f,1.0f).Commit();
       world.Entity().Add<Position>(4.0f,1.0f).Commit();

       world.Entity().Add<Position>(4.0f,1.0f).Add<Velocity>(1.0f).Add<BlaBla>(1.0f).Commit();
*/
                                               
    auto query = world.Query<Position>().Build();

        query.ForEach([](RR::Ecs::EntityId zxc , Position qwe)
            {
                RR::Common::Debug::Log::Format::Info("!!!!! Id {} , {}",zxc.rawId, qwe.x);
    
            });


    // RR::Ecs::TypeId qwe = RR::Ecs::Index<RR::Ecs,((RR::Ecs::HashType(123));
    /*



        const auto s = world.System<Position>("asd").OnEvent<MySuperEvent>().Each([](Position&)
        {
            RR::Common::Debug::Log::Format::Info("!!!!!");
        });

        UNUSED(s); // TODO fix after(s) syntax
        world.System<Position>("asd").OnEvent<MySuperEvent>().After("asd").Each([](Position&)
        {
            RR::Common::Debug::Log::Format::Info("!!!!!");
        });

        world.Tick();

        world.Event<MySuperEvent>().EmitImmediately({});

      //  world.System<Position>().OnEvent<MySuperEvent>().each();

        world.flecs().system<Position>().each(qwe_ES);
       // world.system<Position>().each(qwe_ES);
        world.system<Position>().on_event<MySuperEvent, MySuperEvent>().each(qwe_ES);


        UNUSED(world, ctx);

            // Create observer for custom event
        world.flecs().observer<Position>()
            .event<MyEvent>()
            .each([](flecs::iter& it, size_t i, Position&) {
                 RR::Common::Debug::Log::Format::Info("{}: {}: {}", it.event().name().c_str(), it.event_id().str().c_str(), it.entity(i).name().c_str());
            });

        // The observer query can be matched against the entity, so make sure it
        // has the Position component before emitting the event. This does not
        // trigger the observer yet.
        flecs::entity e = world.flecs().entity("e")
            .set<Position>({10, 20});

             UNUSED(e);
        world.event<MySuperEvent>().entity(e).emit(MySuperEvent{});

        world.flecs().get_alive(e.id()).get([](Position&) {
                RR::Common::Debug::Log::Format::Info("!!!!!");
        });


        flecs::entity g = world.flecs().entity("e!")
            .set<BlaBla>({1});

             UNUSED(g);

        e.get([](Position&) {
                RR::Common::Debug::Log::Format::Info("!!!!!");
        });
    */
    /*
        // Create a system for printing the entity position
         auto s = world.flecs().system<const BlaBla>()
            .kind(flecs::PostUpdate)
            .entity(1)
            .each([&world, e](flecs::entity, const BlaBla&) {
                RR::Common::Debug::Log::Format::Info("It's me!");
                // Emit the custom event
                world.flecs().event<MyEvent>()
                    .id<Position>()
                    .entity(e)
                    .emit();

            });
             UNUSED(s);
    */

    /*
        world.add<TaskBar>();
        ImGui::SetCurrentContext(ctx.imguiCtx);

    world.system("TaskBar").with(EcsSystem).each([](flecs::entity e){
        Log::Format::Info("{}", e.name().c_str());
    });
    */
    // SetAllocatorFunctions();
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

CR_EXPORT int cr_main(struct cr_plugin* ctx, enum cr_op operation)
{
    assert(ctx);
    assert(ctx->userdata);
    switch (operation)
    {
        case CR_LOAD: return init(*(RR::EcsModule::Context*)ctx->userdata); // loading back from a reload
        case CR_UNLOAD: return 0; // preparing to a new reload
        case CR_CLOSE: return 0; // the plugin will close and not reload anymore
        default: return 0; // CR_STEP
    }
}
#pragma clang optimize on