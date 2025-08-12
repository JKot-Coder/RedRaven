#include "Application.hpp"

#include "ecs/Ecs.hpp"

#include "ecs_window/Window.hpp"

namespace RR::App
{
    struct Application
    {
        ECS_SINGLETON;
        static bool quit;
    };

    bool Application::quit = false;

    struct MainWindow{};
    struct Quit : public Ecs::Event
    {
        Quit() : Event(Ecs::GetEventId<Quit>, sizeof(Quit)) { }
    };

    void Init(Ecs::World& world)
    {
        world.System().OnEvent<Quit>().With<Application>().ForEach([]() {
            Application::quit = true;
        });
    }

    int RunApplication()
    {
        Ecs::World world;
        Init(world);
        Ecs::WindowModule::Init(world);

        world.OrderSystems();

        world.Entity().Add<Ecs::WindowModule::Window>().Add<MainWindow>().Apply();
        world.Entity().Add<Application>().Apply();

        while (!Application::quit)
        {
            world.EmitImmediately<Ecs::WindowModule::Tick>({});
            world.Tick();
        }

        return 0;
    }
}
