#include "Application.hpp"

#include "common/Time.hpp"
#include "common/debug/LeakDetector.hpp"

#include "resource_manager/ResourceManager.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/Fence.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

#include "render/RenderContext.hpp"

//#include "windowing/InputtingWindow.hpp"
#include "windowing/WindowSettings.hpp"
#include "windowing/Windowing.hpp"
#include "windowing/Window.hpp"

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
        renderContext.ResetSwapChain(swapChain_, desc);

        index = 0;
    }

    void Application::Start()
    {
        Debug::LeakDetector::Instance();
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
        desciption.gpuResourceFormat = GAPI::GpuResourceFormat::BGRA8Unorm;
        desciption.windowHandle = _window->GetNativeHandle();

        swapChain_ = renderContext.CreateSwapchain(desciption, "Primary");

        auto fence = renderContext.CreateFence("qwe");

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

            // auto index2 = index;
            renderContext.ExecuteAsync(
                [swapChain = swapChain_, index2 = index, commandList](GAPI::Device& device) {
                    std::ignore = device;

                    auto texture = swapChain->GetTexture(index2);
                    //Log::Print::Info("Texture %s\n", texture->GetName());

                    auto rtv = texture->GetRTV();

                    commandList->ClearRenderTargetView(rtv, Vector4(static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX), 0, 0, 0));
                    commandList->Close();
                });

            renderContext.Submit(commandQueue, commandList);

            renderContext.Present(swapChain_);
            renderContext.MoveToNextFrame(commandQueue);

            index = (++index % swapChain_->GetDescription().bufferCount);
            frame++;

            time->Update();
        }

        commandQueue = nullptr;
        commandList = nullptr;

        commandQueue = nullptr;
        commandList = nullptr;
        fence = nullptr;

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
        settings.Width = 800;
        settings.Height = 600;
        settings.Title = "OpenDemo";
        
        Windowing::Windowing::Subscribe(this);
        _window = std::make_shared<Windowing::Window>();
        _window->Init(settings);

       // Inputting::Instance()->Init();
       // Inputting::Instance()->SubscribeToWindow(_window);

        auto& renderContext = Render::RenderContext::Instance();
        renderContext.Init();

        // auto& render = Rendering::Instance();
        // render->Init(_window);
    }

    void Application::terminate()
    {
        swapChain_ = nullptr;

        Render::RenderContext::Instance().Terminate();

        //_scene->Terminate();

        _window.reset();
        _window = nullptr;

       // Inputting::Instance()->Terminate();

        // Rendering::Instance()->Terminate();
        Windowing::Windowing::UnSubscribe(this);
    }

    void Application::loadResouces()
    {
        // _scene = std::make_shared<Scenes::Scene_2>();
        // _scene->Init();
    }
}