#include "Application.hpp"

#include "ecs/Ecs.hpp"

#include "ecs_window/Window.hpp"

namespace RR::App
{
    int RunApplication()
    {
        Ecs::World world;

        Ecs::WindowModule::Init(world);

        world.OrderSystems();

        world.Entity().Add<Ecs::WindowModule::Window>().Apply();

        while (true)
        {
            world.EmitImmediately<Ecs::WindowModule::Tick>({});
            world.Tick();
        }

        return 0;
    }
}
