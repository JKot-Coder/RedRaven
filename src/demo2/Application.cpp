#include "Application.hpp"

#include "ecs/Ecs.hpp"

#include "ecs_window/Window.hpp"

#include "gapi/Device.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/CommandList2.hpp"

#include "math/VectorMath.hpp"

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

    GAPI::SwapChain::UniquePtr CreateSwapChain(Ecs::WindowModule::Window& window)
    {
        GAPI::SwapChainDescription swapChainDescription;
        swapChainDescription.windowNativeHandle = window.nativeHandle;
        // TODO: get window size
        swapChainDescription.width = 1920;
        swapChainDescription.height = 1080;
        swapChainDescription.bufferCount = 2;
        swapChainDescription.gpuResourceFormat = GAPI::GpuResourceFormat::RGBA8UnormSrgb;

        return RenderLoom::DeviceContext::Instance().CreateSwapchain(swapChainDescription);
    }

    int RunApplication()
    {
        Ecs::World world;
        Init(world);
        Ecs::WindowModule::Init(world);

        world.OrderSystems();

        world.Entity().Add<Application>().Apply();


        GAPI::DeviceDescription description;
        auto& deviceContext = RenderLoom::DeviceContext::Instance();
        deviceContext.Init(description);

        GAPI::SwapChain::UniquePtr swapChain;

        auto windowEntity = world.Entity().Add<Ecs::WindowModule::Window>().Add<MainWindow>().Apply();

        world.View().With<Ecs::WindowModule::Window>().ForEntity(windowEntity, [&swapChain](Ecs::WindowModule::Window& window) {
            swapChain = CreateSwapChain(window);
        });

        auto texture = deviceContext.CreateTexture(GAPI::GpuResourceDescription::Texture2D(1920, 1080, GAPI::GpuResourceFormat::RGBA8Unorm, GAPI::GpuResourceBindFlags::RenderTarget), nullptr, "Empty");
        auto ctx = deviceContext.CreateGraphicsCommandContext("test");
        auto commandQueue = deviceContext.CreateCommandQueue(GAPI::CommandQueueType::Graphics, "test");

        while (!Application::quit)
        {
            world.EmitImmediately<Ecs::WindowModule::Tick>({});
            world.Tick();

            ctx->ClearRenderTargetView(texture->GetRTV(), Vector4(1.0f, 1.0f, 0.0f, 1.0f));
            deviceContext.Compile(ctx.get());
            commandQueue->Submit(ctx.get());

            deviceContext.Present(swapChain.get());
            deviceContext.MoveToNextFrame(0);
        }

        return 0;
    }
}
