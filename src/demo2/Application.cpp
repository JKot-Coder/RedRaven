#include "Application.hpp"

#include "ecs/Ecs.hpp"

#include "ecs_window/Window.hpp"

#include "gapi/Device.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Shader.hpp"
#include "gapi/Texture.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/CommandList2.hpp"

#include "math/VectorMath.hpp"

#include "render/DeviceContext.hpp"

namespace RR::App
{

    struct Application
    {
        struct Instance
        {
            bool quit = false;
            GAPI::SwapChain::UniquePtr swapChain;
        };

        ECS_SINGLETON;
        Application(Instance* instance) : instance(instance) { ASSERT(instance); }

        eastl::unique_ptr<Instance> instance;
    };

    struct MainWindow{};
    struct Quit : public Ecs::Event
    {
        Quit() : Event(Ecs::GetEventId<Quit>, sizeof(Quit)) { }
    };

    static const char* VSSource = R"(
        struct PSInput
        {
            float4 Pos   : SV_POSITION;
            float3 Color : COLOR;
        };

        void main(in  uint    VertId : SV_VertexID,
                  out PSInput PSIn)
        {
            float4 Pos[3];
            Pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
            Pos[1] = float4( 0.0, +0.5, 0.0, 1.0);
            Pos[2] = float4(+0.5, -0.5, 0.0, 1.0);

            float3 Col[3];
            Col[0] = float3(1.0, 0.0, 0.0); // red
            Col[1] = float3(0.0, 1.0, 0.0); // green
            Col[2] = float3(0.0, 0.0, 1.0); // blue

            PSIn.Pos   = Pos[VertId];
            PSIn.Color = Col[VertId];
        }
        )";

    // Pixel shader simply outputs interpolated vertex color
    static const char* PSSource = R"(
        struct PSInput
        {
            float4 Pos   : SV_POSITION;
            float3 Color : COLOR;
        };

        struct PSOutput
        {
            float4 Color : SV_TARGET;
        };

        void main(in  PSInput  PSIn,
                  out PSOutput PSOut)
        {
            PSOut.Color = float4(PSIn.Color.rgb, 1.0);
        }
        )";

    void Init(Ecs::World& world)
    {
        world.System()
            .OnEvent<Ecs::WindowModule::Window::OnClose>()
            .With<Ecs::WindowModule::Window>()
            .With<MainWindow>()
            .ForEach([](Ecs::World& world) {
                world.Emit<Quit>({});
            });

        world.System()
            .OnEvent<Ecs::WindowModule::Window::OnResize>()
            .ForEach([](const Ecs::WindowModule::Window::OnResize& event, Ecs::World& world) {
                world.View().With<Application>().ForEach([event](Application& application) {
                    GAPI::SwapChain* swapChain = application.instance->swapChain.get();
                    auto& deviceContext = Render::DeviceContext::Instance();
                    deviceContext.ResizeSwapChain(swapChain, event.width, event.height);
                });
            });

        world.System().OnEvent<Quit>().With<Application>().ForEach([](Application& application) {
            application.instance->quit = true;
        });
    }

    GAPI::SwapChain::UniquePtr CreateSwapChain(Ecs::WindowModule::Window& window, Ecs::WindowModule::WindowDescription& description)
    {
        GAPI::SwapChainDescription swapChainDescription;
        swapChainDescription.windowNativeHandle = window.nativeHandle;

        swapChainDescription.width = description.width;
        swapChainDescription.height = description.height;
        swapChainDescription.bufferCount = 2;
        swapChainDescription.gpuResourceFormat = GAPI::GpuResourceFormat::RGBA8UnormSrgb;

        return Render::DeviceContext::Instance().CreateSwapchain(swapChainDescription);
    }

    int RunApplication()
    {
        Ecs::World world;
        Init(world);
        Ecs::WindowModule::Init(world);

        world.OrderSystems();

        auto* applicationInstance = new Application::Instance();
        world.Entity().Add<Application>(applicationInstance).Apply();

        GAPI::DeviceDescription description;
        auto& deviceContext = Render::DeviceContext::Instance();
        deviceContext.Init(description);

        auto windowEntity = world.Entity().Add<Ecs::WindowModule::Window>().Add<Ecs::WindowModule::WindowDescription>(800, 600).Add<MainWindow>().Apply();

        world.View()
            .With<Ecs::WindowModule::Window>()
            .With<Ecs::WindowModule::WindowDescription>()
            .ForEntity(windowEntity,
                       [applicationInstance](Ecs::WindowModule::Window& window, Ecs::WindowModule::WindowDescription& description) {
                           applicationInstance->swapChain = CreateSwapChain(window, description);
                       });

        auto texture = deviceContext.CreateTexture(GAPI::GpuResourceDescription::Texture2D(1920, 1080, GAPI::GpuResourceFormat::RGBA8Unorm, GAPI::GpuResourceBindFlags::RenderTarget), nullptr, "Empty");
        auto ctx = deviceContext.CreateGraphicsCommandContext("test");
        auto commandQueue = deviceContext.CreateCommandQueue(GAPI::CommandQueueType::Graphics, "test");

        auto shaderVS = deviceContext.CreateShader(GAPI::ShaderDescription(GAPI::ShaderType::Vertex, "main", VSSource), "testVS");
        auto shaderPS = deviceContext.CreateShader(GAPI::ShaderDescription(GAPI::ShaderType::Pixel, "main", PSSource), "testPS");

        while (!applicationInstance->quit)
        {
            world.EmitImmediately<Ecs::WindowModule::Tick>({});
            world.Tick();

            ctx->ClearRenderTargetView(applicationInstance->swapChain->GetCurrentBackBufferTexture()->GetRTV(), Vector4(1.0f, 0.0f, rand() % 255 / 255.0f, 1.0f));
            deviceContext.Compile(ctx.get());
            commandQueue->Submit(ctx.get());

            deviceContext.Present(applicationInstance->swapChain.get());
            deviceContext.MoveToNextFrame(0);
        }

        return 0;
    }
}
