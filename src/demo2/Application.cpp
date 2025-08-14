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

    GAPI::SwapChain::SharedPtr CreateSwapChain(const RenderLoom::DeviceContext& deviceContext, Ecs::WindowModule::Window& window)
    {
        GAPI::SwapChainDescription swapChainDescription;
        swapChainDescription.windowNativeHandle = window.nativeHandle;
        // TODO: get window size
        swapChainDescription.width = 1920;
        swapChainDescription.height = 1080;
        swapChainDescription.bufferCount = 2;
        swapChainDescription.gpuResourceFormat = GAPI::GpuResourceFormat::RGBA8UnormSrgb;

        return deviceContext.CreateSwapchain(swapChainDescription);
    }

    int RunApplication()
    {
        Ecs::World world;
        Init(world);
        Ecs::WindowModule::Init(world);

        world.OrderSystems();

        world.Entity().Add<Application>().Apply();


        GAPI::DeviceDescription description;
        RenderLoom::DeviceContext deviceContext;
        deviceContext.Init(description);

        GAPI::SwapChain::SharedPtr swapChain;

        auto windowEntity = world.Entity().Add<Ecs::WindowModule::Window>().Add<MainWindow>().Apply();

        world.View().With<Ecs::WindowModule::Window>().ForEntity(windowEntity, [&deviceContext, &swapChain](Ecs::WindowModule::Window& window) {
            swapChain = CreateSwapChain(deviceContext, window);
        });

        while (!Application::quit)
        {
            world.EmitImmediately<Ecs::WindowModule::Tick>({});
            world.Tick();

            deviceContext.Present(swapChain);
            deviceContext.MoveToNextFrame(0);
        }

        return 0;
    }
}
