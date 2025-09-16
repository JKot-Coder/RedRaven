#include "Application.hpp"

// #include <Windows.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/Texture.hpp"
#include "gapi/SwapChain.hpp"
#include "render/DeviceContext.hpp"
#include "math/Math.hpp"

#include "platform/Toolkit.hpp"
#include "platform/Window.hpp"

#include "common/Result.hpp"
#include "common/RingBuffer.hpp"
#include "ecs_imgui/ImGui.hpp"

namespace RR
{
    namespace
    {
        //------------------------------------------------------------------------------------------------
        struct CD3DX12_CLEAR_VALUE : public D3D12_CLEAR_VALUE
        {
            CD3DX12_CLEAR_VALUE() = default;
            explicit CD3DX12_CLEAR_VALUE(const D3D12_CLEAR_VALUE& o) noexcept : D3D12_CLEAR_VALUE(o)
            {
            }
            CD3DX12_CLEAR_VALUE(
                DXGI_FORMAT format,
                const FLOAT color[4]) noexcept
            {
                Format = format;
                memcpy(Color, color, sizeof(Color));
            }
            CD3DX12_CLEAR_VALUE(
                DXGI_FORMAT format,
                FLOAT depth,
                UINT8 stencil) noexcept
            {
                Format = format;
                memset(&Color, 0, sizeof(Color));
                /* Use memcpy to preserve NAN values */
                memcpy(&DepthStencil.Depth, &depth, sizeof(depth));
                DepthStencil.Stencil = stencil;
            }
        };

        // Forward declarations of helper functions
        bool CreateDeviceD3D(const Platform::Window::SharedPtr&);
        void CleanupDeviceD3D();

        static bool g_done = false;
        static GAPI::CommandQueue::UniquePtr g_CommandQueue = nullptr;
        static GAPI::SwapChain::UniquePtr g_pSwapChain;

        // Helper functions
        bool CreateDeviceD3D(const Platform::Window::SharedPtr& window)
        {
            auto& deviceContext = Render::DeviceContext::Instance();
            GAPI::DeviceDesc deviceDesc;
            deviceDesc.debugMode = GAPI::DeviceDesc::DebugMode::Retail;
            deviceDesc.maxFramesInFlight = 1;

            if(!deviceContext.Init(deviceDesc))
            {
                LOG_ERROR("Failed to initialize device context");
                return false;
            }

            GAPI::SwapChainDesc desciption = {};
            desciption.width = window->GetSize().x;
            desciption.height = window->GetSize().y;
            desciption.bufferCount = 2;
            desciption.isStereo = false;
            desciption.gpuResourceFormat = GAPI::GpuResourceFormat::RGBA8Unorm;
            desciption.windowNativeHandle = window->GetNativeHandleRaw();

            g_pSwapChain = deviceContext.CreateSwapchain(desciption);

            return true;
        }

        void CleanupDeviceD3D()
        {
            g_pSwapChain = nullptr;
            g_CommandQueue = nullptr;
        }
    }

    void CloseCallback(const Platform::Window&)
    {
        g_done = true;
    }

    /*void Application::resizeCallback(const Platform::Window&, const Vector2i& size)
    {
        auto& deviceContext = Render::DeviceContext::Instance();
        deviceContext.ResizeSwapChain(g_pSwapChain.get(), size.x, size.y);

        draw(deviceContext, 0.00001f);
    }*/

    Application::Application() {}
    Application::~Application() {}

    int Application::Run()
    {
        init();

        auto& platformToolkit = Platform::Toolkit::Instance();
        platformToolkit.Init();

        Platform::Window::Description windowDesc;
        windowDesc.size = {800, 600};
        windowDesc.title = "Demo";

        // Create window with graphics context
        auto window = platformToolkit.CreatePlatformWindow(windowDesc);
        if (!window)
            return 1;

       // window->OnResize.Subscribe<Application, &Application::resizeCallback>(this);
        window->OnClose.Subscribe<CloseCallback>();

        // Initialize Direct3D
        if (!CreateDeviceD3D(window))
        {
            CleanupDeviceD3D();
            return 1;
        }
/*
        ImGuiIO& io = ImGui::GetIO();

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        //   io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
        io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
        io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;

        io.ConfigDockingTransparentPayload = true;
        // io.ConfigViewportsNoAutoMerge = true;
        // io.ConfigViewportsNoTaskBarIcon = true;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsClassic();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Setup Platform/Renderer backends
        imguiPlatformInput.Init(window, true);*/
/*
        auto& deviceContext = Render::DeviceContext::Instance();
        ID3D12Device* rawDevice;
        deviceContext.ExecuteAwait([&rawDevice](RR::GAPI::Device& device) { rawDevice = std::any_cast<ID3D12Device*>(device.GetRawDevice()); });

        ImGui_ImplDX12_Init(rawDevice, NUM_FRAMES_IN_FLIGHT,
                            DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
                            g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
                            g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        // io.Fonts->AddFontDefault();
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
        // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
        // IM_ASSERT(font != nullptr);

        // Main loop
        const auto& toolkit = Platform::Toolkit::Instance();
        float dt = 1.0f / 60.0f;
        float last_time = (float)toolkit.GetTime();
        while (!g_done)
        {
            Platform::Toolkit::Instance().PoolEvents();

            draw(deviceContext, Max(dt, 0.00001f));

            float current_time = (float)toolkit.GetTime();
            dt = (current_time - last_time);
            last_time = current_time;
        }

        WaitForLastSubmittedFrame();

        // Cleanup
        ImGui_ImplDX12_Shutdown();
        imguiPlatformInput.Shutdown();
        ImGui::DestroyContext();
*/
        CleanupDeviceD3D();

        Render::DeviceContext::Instance().Terminate();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        window = nullptr;

        return 0;
    }
    #pragma clang optimize off
    void Application::draw(Render::DeviceContext& deviceContext, float dt)
    {
        UNUSED(dt);

        //ImGui_ImplDX12_NewFrame();
        //imguiPlatformInput.NewFrame(dt);

        ImGuiEcs::Draw(editorWorld);
/*
        static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

                D3D12_RESOURCE_BARRIER barrier = {};
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx];
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

                const auto& сommandList = g_CommandLists.Advance();

                // Render Dear ImGui graphics
                сommandList->ClearRenderTargetView(g_frameBuffers[backBufferIdx]->GetDescription().renderTargetViews[0], Vector4(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w));


                frameIdx++;
                ID3D12GraphicsCommandList4* g_pd3dCommandList = std::any_cast<ID3D12GraphicsCommandList4*>(сommandList->GetNativeHandle());

                g_pd3dCommandList->ResourceBarrier(1, &barrier);
                сommandList->SetFrameBuffer(g_frameBuffers[backBufferIdx]);

                g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);

                ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);

                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

                g_pd3dCommandList->ResourceBarrier(1, &barrier);
                сommandList->Close();


        //    });

        swindex = (++swindex % NUM_BACK_BUFFERS);

        deviceContext.Submit(g_CommandQueue, сommandList);

        ImGuiIO& io = ImGui::GetIO();
        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            // ImGui::UpdatePlatformWindows();
            //   ImGui::RenderPlatformWindowsDefault(nullptr, (void*)g_pd3dCommandList);
        }
*/
        deviceContext.Present(g_pSwapChain.get());
        deviceContext.MoveToNextFrame(0);
    }


    void Application::init()
    {
        // Setup Dear ImGui context
      //  IMGUI_CHECKVERSION();
     //   auto imguiCtx = ImGui::CreateContext();

      //  EcsModule::Context ctx(editorWorld, *imguiCtx);


     //   ecsManager = std::make_unique<EcsModule::Manager>(ctx);
      /*  if (ecsManager->Load("editor_ecs_d.dll") != Common::RResult::Ok)
        {
            LOG_ERROR("cant load shit!");
        }
        else
        {
        }*/
        editorWorld.OrderSystems();
     //  ImGuiEcs::Init(editorWorld, ImGui::GetCurrentContext());

        editorWorld.OrderSystems();

        //   testEvent.Register<Application>(this, &Application::bar);
    }
}