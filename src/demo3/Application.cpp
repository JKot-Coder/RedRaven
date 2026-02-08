#include "Application.hpp"

#include "ecs/Ecs.hpp"

#include "ecs_window/Window.hpp"

#include "gapi/Device.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Shader.hpp"
#include "gapi/Texture.hpp"
#include "gapi/Buffer.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/CommandList.hpp"
#include "gapi/PipelineState.hpp"
#include "gapi/RenderPassDesc.hpp"

#include "math/VectorMath.hpp"

#include "render/ResourcePointers.hpp"
#include "render/VertexFormats/Vertex.hpp"
#include "render/DeviceContext.hpp"
#include "render/CommandEncoder.hpp"
#include "render/EffectManager.hpp"
#include "render/VertexFormats/Vertex.hpp"

#include "common/Result.hpp"

namespace RR::App
{

    constexpr uint32_t BACK_BUFFERS_COUNT = 2;

    struct Application
    {
        struct Instance
        {
            bool quit = false;
            Render::SwapChainUniquePtr swapChain;
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
            .OnEvent<Ecs::WindowModule::Window::OnResizeFinished>()
            .ForEach([](const Ecs::WindowModule::Window::OnResizeFinished& event, Ecs::World& world) {
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

    Render::SwapChainUniquePtr CreateSwapChain(Ecs::WindowModule::Window& window, Ecs::WindowModule::WindowDesc& description)
    {
        GAPI::SwapChainDesc swapChainDesc;
        swapChainDesc.windowNativeHandle = window.nativeHandle;

        swapChainDesc.width = description.width;
        swapChainDesc.height = description.height;
        swapChainDesc.backBuffersCount = BACK_BUFFERS_COUNT;
        swapChainDesc.presentMode = GAPI::SwapChainDesc::PresentMode::Fifo;
        swapChainDesc.backBufferFormat = GAPI::GpuResourceFormat::BGRA8Unorm;

        return Render::DeviceContext::Instance().CreateSwapchain(swapChainDesc);
    }

    GAPI::Buffer::SharedPtr CreateVertexBuffer()
    {
        // Layout of this structure matches the one we defined in the pipeline state
        struct Vertex
        {
            Vector4 pos;
            Vector4 color;
        };

        // Cube vertices

        //      (-1,+1,+1)________________(+1,+1,+1)
        //               /|              /|
        //              / |             / |
        //             /  |            /  |
        //            /   |           /   |
        //(-1,-1,+1) /____|__________/(+1,-1,+1)
        //           |    |__________|____|
        //           |   /(-1,+1,-1) |    /(+1,+1,-1)
        //           |  /            |   /
        //           | /             |  /
        //           |/              | /
        //           /_______________|/
        //        (-1,-1,-1)       (+1,-1,-1)
        //

        constexpr Vertex CubeVerts[8] =
        {
            {Vector4{-1, -1, -1, 1}, Vector4{1, 0, 0, 1}},
            {Vector4{-1, +1, -1, 1}, Vector4{0, 1, 0, 1}},
            {Vector4{+1, +1, -1, 1}, Vector4{0, 0, 1, 1}},
            {Vector4{+1, -1, -1, 1}, Vector4{1, 1, 1, 1}},

            {Vector4{-1, -1, +1, 1}, Vector4{1, 1, 0, 1}},
            {Vector4{-1, +1, +1, 1}, Vector4{0, 1, 1, 1}},
            {Vector4{+1, +1, +1, 1}, Vector4{1, 0, 1, 1}},
            {Vector4{+1, -1, +1, 1}, Vector4{0.2f, 0.2f, 0.2f, 1.f}},
        };

        auto& deviceContext = Render::DeviceContext::Instance();
        GAPI::BufferData bufferData = { CubeVerts, sizeof(CubeVerts) };
        return deviceContext.CreateBuffer(GAPI::GpuResourceDesc::Buffer(sizeof(CubeVerts)), &bufferData, "Cube vertex buffer");
    }

    GAPI::Buffer::SharedPtr CreateIndexBuffer()
    {
        // clang-format off
        constexpr uint32_t Indices[] =
        {
            2,0,1, 2,3,0,
            4,6,5, 4,7,6,
            0,7,4, 0,3,7,
            1,0,4, 1,4,5,
            1,5,2, 5,6,2,
            3,6,7, 3,2,6
        };
        // clang-format on
        auto& deviceContext = Render::DeviceContext::Instance();
        GAPI::BufferData bufferData = { Indices, sizeof(Indices) };
        return deviceContext.CreateBuffer(GAPI::GpuResourceDesc::IndexBuffer(sizeof(Indices) / sizeof(Indices[0]), GAPI::GpuResourceFormat::R32Uint), &bufferData, "Cube index buffer");
    }

    Render::GraphicPipelineStateUniquePtr CreatePipelineState(GAPI::Shader* vs, GAPI::Shader* ps)
    {
        auto& deviceContext = Render::DeviceContext::Instance();

      /* // Pipeline state object encompasses configuration of all GPU stages
        GAPI::GraphicPipelineStateDesc PSOCreateInfo;

        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        PSOCreateInfo.PSODesc.Name = "Simple triangle PSO";

        // This is a graphics pipeline
        PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        // clang-format off
        // This tutorial will render to a single render target
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 1;
        // Set render target format which is the format of the swap chain's color buffer
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                = m_pSwapChain->GetDesc().ColorBufferFormat;
        // Use the depth buffer format from the swap chain
        PSOCreateInfo.GraphicsPipeline.DSVFormat                    = m_pSwapChain->GetDesc().DepthBufferFormat;
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology            = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // No back face culling for this tutorial
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_NONE;
        // Disable depth testing
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
        // clang-format on

        ShaderCreateInfo ShaderCI;
        // Tell the system that the shader source code is in HLSL.
        // For OpenGL, the engine will convert this into GLSL under the hood.
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        ShaderCI.Desc.UseCombinedTextureSamplers = true;


        PSOCreateInfo.pVS = shaderVS;
        PSOCreateInfo.pPS = shaderPS;
        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSO);
        return m_pPSO;
        return nullptr;*/

        GAPI::GraphicPipelineStateDesc desc;

        desc.colorAttachmentCount = 1;
        desc.colorAttachmentFormats[0] = GAPI::GpuResourceFormat::RGBA8UnormSrgb;
        desc.depthStencilFormat = GAPI::GpuResourceFormat::D32Float;

        ASSERT(vs);
        ASSERT(ps);
        desc.vs = vs;
        desc.ps = ps;

        auto pipelineState = deviceContext.CreatePipelineState(desc, "test");
        return pipelineState;
    }

    int RunApplication()
    {
        Ecs::World world;
        Init(world);
        Ecs::WindowModule::Init(world);

        world.OrderSystems();

        auto* applicationInstance = new Application::Instance();
        world.Entity().Add<Application>(applicationInstance).Apply();

        GAPI::DeviceDesc description;
        auto& deviceContext = Render::DeviceContext::Instance();
        if(!deviceContext.Init(description))
        {
            LOG_ERROR("Failed to initialize device context");
            return 1;
        }

        Render::EffectManager::Instance();
        if(RR_FAILED(Render::EffectManager::Instance().Init("CompiledShaders.rfxlib")))
        {
            LOG_ERROR("Failed to load effects library CompiledShaders.rfxlib");
            return 1;
        }

       auto triangleEffect = Render::EffectManager::Instance().Load("triangle");
         UNUSED(triangleEffect);
        //auto cubeEffect = Render::EffectManager::Instance().Load("cube");

        auto windowEntity = world.Entity().Add<Ecs::WindowModule::Window>().Add<Ecs::WindowModule::WindowDesc>(800, 600).Add<MainWindow>().Apply();

        GAPI::SwapChain* swapChain = nullptr;
        world.View()
            .With<Ecs::WindowModule::Window>()
            .With<Ecs::WindowModule::WindowDesc>()
            .ForEntity(windowEntity,
                        [applicationInstance, &swapChain](Ecs::WindowModule::Window& window, Ecs::WindowModule::WindowDesc& description) {
                           applicationInstance->swapChain = CreateSwapChain(window, description);
                           swapChain = applicationInstance->swapChain.get();
                       });

        auto texture = deviceContext.CreateTexture(GAPI::GpuResourceDesc::Texture2D(1920, 1080, GAPI::GpuResourceFormat::RGBA8Unorm, GAPI::GpuResourceBindFlags::RenderTarget), nullptr, "Empty");
        auto ctx = deviceContext.CreateCommandEncoder("test");
         auto commandQueue = deviceContext.CreateCommandQueue(GAPI::CommandQueueType::Graphics, "test");

        auto vertexBuffer = CreateVertexBuffer();
        auto indexBuffer = CreateIndexBuffer();

        while (!applicationInstance->quit)
        {
            world.EmitImmediately<Ecs::WindowModule::Tick>({});
            world.Tick();


/*
            auto bindingContext = ctx->GetBindingContext();

            bindGroup = bindingContext->GetBindGroup(effect, "uniforms");
            bindGrour["someShit"] = 123;


            bindingContext->SetBindGroup("qwe", globalUniforms);*/


           /* const auto renderPassDesc = GAPI::RenderPassDesc::Builder()
                                            .ColorAttachment(0, swapChain->GetCurrentBackBufferTexture()->GetRTV(), GAPI::AttachmentLoadOp::Clear, Vector4(1.0f, 1.0f, rand() % 255 / 255.0f, 1.0f))
                                            .Build();
                                            UNUSED(renderPassDesc);

            auto renderPassEncoder = ctx->BeginRenderPass(renderPassDesc);



            renderPassEncoder.Draw(triangleEffect.get(), GAPI::PrimitiveTopology::TriangleList, 0, 3);

            renderPassEncoder.End();*/

            ctx->Finish();


          //  ctx->SetVertexLayout(&Render::Vertex::GetVertexLayout());
          // ctx->SetIndexBuffer(indexBuffer.get());
          //  ctx->SetVertexBuffer(0, *vertexBuffer.get(), 0);
          //  ctx->DrawIndexed(cubeEffect.get(), GAPI::PrimitiveTopology::TriangleList, 0, 36);*/

            deviceContext.Compile(*ctx);
            deviceContext.Submit(commandQueue.get(), *ctx);
            deviceContext.Present(applicationInstance->swapChain.get());
            deviceContext.MoveToNextFrame(0);
        }

        deviceContext.Terminate();

        return 0;
    }
}
