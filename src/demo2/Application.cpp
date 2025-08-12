#include "Application.hpp"

#include "ecs/Ecs.hpp"

namespace RR::App
{
    int Application::Run()
    {
        Ecs::World world;

        while (true)
        {
            world.Tick();
        }

        return 0;
    }
}
