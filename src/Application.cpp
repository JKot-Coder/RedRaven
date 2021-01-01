#include "Application.hpp"

#include "common/Time.hpp"

#include "inputting/Input.hpp"

#include "resource_manager/ResourceManager.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/Fence.hpp"
#include "gapi/ResourceViews.hpp"
#include "gapi/Result.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

#include "render/RenderContext.hpp"

#include "rendering/Mesh.hpp"
#include "rendering/Primitives.hpp"
#include "rendering/Render.hpp"
#include "rendering/RenderPipeline.hpp"
#include "rendering/Shader.hpp"

#include "windowing/InputtingWindow.hpp"
#include "windowing/WindowSettings.hpp"
#include "windowing/Windowing.hpp"

#include "scenes/Scene_1.hpp"
#include "scenes/Scene_2.hpp"

namespace OpenDemo
{
    using namespace Common;

    static uint32_t index = 0;
    static uint32_t frame = 0;

    void Application::OnWindowResize(const Windowing::Window& window_)
    {
        GAPI::SwapChainDescription desc = swapChain_->GetDescription();
        desc.width = window_.GetWidth();
        desc.height = window_.GetHeight();

        auto& renderContext = Render::RenderContext::Instance();
        const auto result = renderContext.ResetSwapChain(swapChain_, desc);
        ASSERT(result)

        index = 0;
    }

    void Application::Start()
    {
        init();

        /*    const auto cmdList = new GAPI::CommandList("asd");
        std::ignore = cmdList;

        auto alloc = cmdList->GetAllocator();
        for (int i = 0; i < 100; i++)
        {
            alloc->emplace_back<GAPI::CommandClearRenderTarget>(GAPI::RenderTargetView::SharedPtr(nullptr),Vector4(0,0,0,0));
        }
        */

        //  const auto& input = Inputting::Instance();
        const auto& time = Time::Instance();
        //   const auto& render = Rendering::Instance();
        //  auto* renderPipeline = new Rendering::`(_window);
        //  renderPipeline->Init();

        time->Init();

        auto& renderContext = Render::RenderContext::Instance();

        auto commandQueue = renderContext.CreteCommandQueue(GAPI::CommandQueueType::Graphics, u8"Primary");
        auto commandList = renderContext.CreateGraphicsCommandList(u8"qwew");
        //  ASSERT(commandList)
        // commandList->Close();

        GAPI::SwapChainDescription desciption = {};
        desciption.width = 100;
        desciption.height = 100;
        desciption.bufferCount = 2;
        desciption.isStereo = false;
        desciption.resourceFormat = GAPI::ResourceFormat::BGRA8Unorm;
        desciption.windowHandle = _window->GetNativeHandle();

        swapChain_ = renderContext.CreateSwapchain(desciption, "Primary");
        auto fence = renderContext.CreateFence("qwe");

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10s);

        while (!_quit)
        {
            Windowing::Windowing::PoolEvents();

            // renderContext.Submit(commandQueue, commandList);

            //  renderPipeline->Collect(_scene);
            //    renderPipeline->Draw();

            //   _scene->Update();
            //device->Present();
            //    render->SwapBuffers();
            //  input->Update();

            // renderContext.Present();

            auto index2 = index;
            renderContext.ExecuteAsync(
                [this, index2, &commandList](GAPI::Device& device) {
                    std::ignore = device;

                    auto texture = swapChain_->GetTexture(index2);
                    ASSERT(texture)
                    //Log::Print::Info("Texture %s\n", texture->GetName());

                    auto rtv = texture->GetRTV();
                    ASSERT(rtv);

                    commandList->ClearRenderTargetView(rtv, Vector4(static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX), 0, 0, 0));
                    commandList->Close();

                    return GAPI::Result::Ok;
                });

            renderContext.Submit(commandQueue, commandList);

            renderContext.Present(swapChain_);
            ASSERT(renderContext.MoveToNextFrame(commandQueue));

            uint64_t cpu = 0;
            uint64_t gpu = 0;

            renderContext.ExecuteAsync(
                [&commandList, &fence, &commandQueue, &cpu, &gpu](GAPI::Device& device) {
                    std::ignore = device;
                    //   device.WaitForGpu();

                    // Schedule a Signal command in the queue.
                    fence->Signal(commandQueue);


                    if (fence->GetCpuValue() >= 2)
                    {
                        // GPU ahead. Throttle cpu.
                        // TODO SubmissionThreadAheadFrames rename/replace
                        fence->SyncCPU(fence->GetCpuValue() - 2, INFINITE);
                    }

                    cpu = fence->GetCpuValue();
                    gpu = fence->GetGpuValue();

                    commandList->Reset();

                    return GAPI::Result::Ok;
                });

            index = (++index % swapChain_->GetDescription().bufferCount);
            frame++;

            time->Update();
        }

        //  delete renderPipeline;

        terminate();
    }

    void Application::OnQuit()
    {
        _quit = true;
    }

    void Application::init()
    {
        Windowing::WindowSettings settings;
        Windowing::WindowRect rect(Windowing::WindowRect::WINDOW_POSITION_CENTERED,
            Windowing::WindowRect::WINDOW_POSITION_CENTERED, 800, 600);

        settings.Title = "OpenDemo";
        settings.WindowRect = rect;

        Windowing::Windowing::Subscribe(this);
        _window = std::shared_ptr<Windowing::InputtingWindow>(new Windowing::InputtingWindow());
        _window->Init(settings, true);

        Inputting::Instance()->Init();
        Inputting::Instance()->SubscribeToWindow(_window);

        auto& renderContext = Render::RenderContext::Instance();
        const auto result = renderContext.Init();

        if (!result)
            Log::Print::Fatal("Fatal error initialize render context with error: %s\n", result.ToString());

        // auto& render = Rendering::Instance();
        // render->Init(_window);
    }

    void Application::terminate()
    {
        Render::RenderContext::Instance().Terminate();

        //_scene->Terminate();

        _window.reset();
        _window = nullptr;

        Inputting::Instance()->Terminate();

        // Rendering::Instance()->Terminate();
        Windowing::Windowing::UnSubscribe(this);
    }

    void Application::loadResouces()
    {
        _scene = std::make_shared<Scenes::Scene_2>();
        _scene->Init();
    }
}