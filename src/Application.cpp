#include "Application.hpp"

#include "common/Time.hpp"

#include "inputting/Input.hpp"

#include "resource_manager/ResourceManager.hpp"

#include "gapi/RenderContext.hpp"

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

    void Application::OnWindowResize(const Windowing::Window& window_)
    {
        std::ignore = window_;
        /* int width = window_.GetWidth();
        int height = window_.GetHeight();
       
        Render::PresentOptions presentOptions;
        presentOptions.bufferCount = 2;
        presentOptions.isStereo = false;
        presentOptions.rect = AlignedBox2i(Vector2i(0, 0), Vector2i(width, height));
        presentOptions.Resource::Format = Render::Resource::Format::Unknown;
        presentOptions.windowHandle = _window->GetNativeHandle();

        submission_->ResetDevice(presentOptions);*/
    }

    void Application::Start()
    {
        init();

        /*    const auto cmdList = new Render::CommandList("asd");
        std::ignore = cmdList;

        auto alloc = cmdList->GetAllocator();
        for (int i = 0; i < 100; i++)
        {
            alloc->emplace_back<Render::CommandClearRenderTarget>(Render::RenderTargetView::SharedPtr(nullptr),Vector4(0,0,0,0));
        }
        */

        //  const auto& input = Inputting::Instance();
        const auto& time = Time::Instance();
        //   const auto& render = Rendering::Instance();
        //  auto* renderPipeline = new Rendering::`(_window);
        //  renderPipeline->Init();

        time->Init();

        auto& renderContext = Render::RenderContext::Instance();
        auto rcc = renderContext.CreateRenderCommandContext(u8"qwew");
        ASSERT(rcc)

        const auto& desc = Render::Texture::Description::Create2D(100, 100, Render::Resource::Format::R8Unorm);
        auto texture = renderContext.CreateTexture(desc, Render::Texture::BindFlags::ShaderResource);
        ASSERT(texture)

        auto rtv = texture->GetRTV();


        while (!_quit)
        {
            Windowing::Windowing::PoolEvents();

            rcc->Reset();
         //   rcc->ClearRenderTargetView(rtv, Vector4(1, 1, 1, 1));
            rcc->Close();

            //  renderPipeline->Collect(_scene);
            //    renderPipeline->Draw();

            //   _scene->Update();
            //device->Present();
            //    render->SwapBuffers();
            //  input->Update();

           // renderContext->Present();

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

        Render::PresentOptions presentOptions;
        presentOptions.bufferCount = 2;
        presentOptions.isStereo = false;
        presentOptions.rect = AlignedBox2i(Vector2i(0, 0), Vector2i(100, 100));
        presentOptions.resourceFormat = Render::Resource::Format::BGRA8Unorm;
        presentOptions.windowHandle = _window->GetNativeHandle();

        auto& renderContext = Render::RenderContext::Instance();
        const auto result = renderContext.Init(presentOptions);

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