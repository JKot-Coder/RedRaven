#include "Application.hpp"

#include "ecs/Ecs.hpp"

#include "ecs_window/Window.hpp"

namespace RR::App
{
    int Application::Run()
    {
        Ecs::World world;

        Ecs::WindowModule::Init(world);

        while (true)
        {
            world.Tick();
        }

        return 0;
    }
}
