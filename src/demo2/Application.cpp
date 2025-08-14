#include "Application.hpp"

#include "ecs/Ecs.hpp"

#include "ecs_window/Window.hpp"

#include "gapi/Device.hpp"
#include "gapi/SwapChain.hpp"
#include "render_loom/DeviceContext.hpp"

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
        world.System()
            .OnEvent<Ecs::WindowModule::Window::Close>()
            .With<Ecs::WindowModule::Window>()
            .With<MainWindow>()
            .ForEach([](Ecs::World& world) {
                world.Emit<Quit>({});
            });

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

        world.Entity().Add<Application>().Apply();

        auto windowEntity = world.Entity().Add<Ecs::WindowModule::Window>().Add<MainWindow>().Apply();
        world.View().With<Ecs::WindowModule::Window>().ForEntity(windowEntity, [](Ecs::WindowModule::Window& window) {
            GAPI::DeviceDescription description;
            RenderLoom::DeviceContext deviceContext;
            deviceContext.Init(description);

            GAPI::SwapChainDescription swapChainDescription;
            swapChainDescription.windowNativeHandle = window.nativeHandle;
            // TODO: get window size
            swapChainDescription.width = 1920;
            swapChainDescription.height = 1080;
            swapChainDescription.bufferCount = 2;
            swapChainDescription.gpuResourceFormat = GAPI::GpuResourceFormat::RGBA8UnormSrgb;

            auto swapChain = deviceContext.CreateSwapchain(swapChainDescription);
        });

        while (!Application::quit)
        {
            world.EmitImmediately<Ecs::WindowModule::Tick>({});
            world.Tick();
        }

        return 0;
    }
}
