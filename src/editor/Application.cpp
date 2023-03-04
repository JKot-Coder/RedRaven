#include "Application.hpp"

//#include <Windows.h>

#include "imgui_impl/ImguiPlatformImpl.hpp"
#include "imgui_impl/ImguiRenderImpl.hpp"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <imgui.h>
#include <tchar.h>

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/Framebuffer.hpp"
#include "gapi/Texture.hpp"
#include "render/DeviceContext.hpp"

#include "platform/Toolkit.hpp"
#include "platform/Window.hpp"

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
                const FLOAT color[4])
            noexcept
            {
                Format = format;
                memcpy(Color, color, sizeof(Color));
            }
            CD3DX12_CLEAR_VALUE(
                DXGI_FORMAT format,
                FLOAT depth,
                UINT8 stencil)
            noexcept
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
        void CreateRenderTarget();
        void CleanupRenderTarget();
        void WaitForLastSubmittedFrame();

        // Data
        static int const NUM_FRAMES_IN_FLIGHT = 3;
        static UINT g_frameIndex = 0;
        static bool g_done = false;
        static uint32_t swindex = 0;
        static int const NUM_BACK_BUFFERS = 3;
        static ID3D12Device* g_pd3dDevice = nullptr;
        static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
        static GAPI::CommandQueue::SharedPtr g_CommandQueue = nullptr;
        static GAPI::GraphicsCommandList::SharedPtr g_CommandList = nullptr;
        static ID3D12GraphicsCommandList4* g_pd3dCommandList = nullptr;
        static GAPI::SwapChain::SharedPtr g_pSwapChain;
        static std::array<GAPI::Framebuffer::SharedPtr, NUM_BACK_BUFFERS> g_frameBuffers;
        static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};

        // Helper functions
        bool CreateDeviceD3D(const Platform::Window::SharedPtr& window)
        {
            auto& deviceContext = Render::DeviceContext::Instance();
            deviceContext.Init();
            deviceContext.ExecuteAwait([](GAPI::Device& device)
                                       { g_pd3dDevice = std::any_cast<ID3D12Device*>(device.GetRawDevice()); });

            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                desc.NumDescriptors = 1;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
                    return false;
            }
            g_CommandQueue = deviceContext.CreteCommandQueue(GAPI::CommandQueueType::Graphics, "Primary");

            g_CommandList = deviceContext.CreateGraphicsCommandList("Primary");
            g_pd3dCommandList = std::any_cast<ID3D12GraphicsCommandList4*>(g_CommandList->GetNativeHandle());
            //  g_pd3dCommandList->Close();

            GAPI::SwapChainDescription desciption = {};
            desciption.width = window->GetSize().x;
            desciption.height = window->GetSize().y;
            desciption.bufferCount = NUM_BACK_BUFFERS;
            desciption.isStereo = false;
            desciption.gpuResourceFormat = GAPI::GpuResourceFormat::RGBA8Unorm;
            desciption.window = window;

            g_pSwapChain = deviceContext.CreateSwapchain(desciption, "Primary");

            CreateRenderTarget();
            return true;
        }

        void CleanupDeviceD3D()
        {
            CleanupRenderTarget();
            g_CommandList = nullptr;
            g_pd3dCommandList = nullptr;
            g_pSwapChain = nullptr;
            g_CommandQueue = nullptr;

            if (g_pd3dSrvDescHeap)
            {
                g_pd3dSrvDescHeap->Release();
                g_pd3dSrvDescHeap = nullptr;
            }
        }

        void CreateRenderTarget()
        {
            auto& deviceContext = Render::DeviceContext::Instance();
            for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
            {
                ID3D12Resource* pBackBuffer = nullptr;
                pBackBuffer = std::any_cast<ID3D12Resource*>(g_pSwapChain->GetBackBufferTexture(i)->GetRawHandle());
                g_mainRenderTargetResource[i] = pBackBuffer;

                GAPI::FramebufferDesc desc = GAPI::FramebufferDesc::Make().BindColorTarget(0, g_pSwapChain->GetBackBufferTexture(i)->GetRTV());
                g_frameBuffers[i] = deviceContext.CreateFramebuffer(desc);
            }
        }

        void CleanupRenderTarget()
        {
            WaitForLastSubmittedFrame();

            for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
                if (g_mainRenderTargetResource[i])
                {
                    g_mainRenderTargetResource[i]->Release();
                    g_mainRenderTargetResource[i] = nullptr;
                    g_frameBuffers[i] = nullptr;
                }
        }

        void WaitForLastSubmittedFrame()
        {
            Render::DeviceContext::Instance().WaitForGpu(g_CommandQueue);
        }
    }

    void CloseCallback(const Platform::Window&)
    {
        g_done = true;
    }

    void ResizeCallback(const Platform::Window&, const Vector2i& size)
    {
        WaitForLastSubmittedFrame();
        CleanupRenderTarget();

        GAPI::SwapChainDescription desc = g_pSwapChain->GetDescription();
        desc.width = size.x;
        desc.height = size.y;

        auto& deviceContext = Render::DeviceContext::Instance();
        deviceContext.ResetSwapChain(g_pSwapChain, desc);

        swindex = 0;

        CreateRenderTarget();
    }

    int Application::Run()
    {
        init();

        auto& platformToolkit = Platform::Toolkit::Instance();
        platformToolkit.Init();

        Platform::Window::Description windowDesc;
        windowDesc.size = { 800, 600 };
        windowDesc.title = "Demo";

        // Create window with graphics context
        auto window = platformToolkit.CreatePlatformWindow(windowDesc);
        if (!window)
            return 1;

        window->OnResize.Subscribe<ResizeCallback>();
        window->OnClose.Subscribe<CloseCallback>();

        // Initialize Direct3D
        if (!CreateDeviceD3D(window))
        {
            CleanupDeviceD3D();
            return 1;
        }

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        //   io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
        io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
        io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;

        io.ConfigDockingTransparentPayload = true;
        //io.ConfigViewportsNoAutoMerge = true;
        //io.ConfigViewportsNoTaskBarIcon = true;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImguiPlatfomImpl imguiPlatformInput;
        // Setup Platform/Renderer backends
        imguiPlatformInput.Init(window, true);

        auto& deviceContext = Render::DeviceContext::Instance();
        ImGui_ImplDX12_Init(deviceContext, NUM_FRAMES_IN_FLIGHT,
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
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
        //IM_ASSERT(font != nullptr);

        // Our state
        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        // Main loop
        const auto& toolkit = Platform::Toolkit::Instance();
        float dt = 1.0f / 60.0f;
        float last_time = (float)toolkit.GetTime();
        while (!g_done)
        {
            Platform::Toolkit::Instance().PoolEvents();

            deviceContext.ExecuteAsync(
                [&](GAPI::Device& device)
                {
                    std::ignore = device;

                    // Start the Dear ImGui frame
                    ImGui_ImplDX12_NewFrame();
                    imguiPlatformInput.NewFrame(dt);

                    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
                    if (show_demo_window)
                        ImGui::ShowDemoWindow(&show_demo_window);

                    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
                    {
                        static float f = 0.0f;
                        static int counter = 0;

                        ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

                        ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
                        ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
                        ImGui::Checkbox("Another Window", &show_another_window);

                        ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
                        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

                        if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                            counter++;
                        ImGui::SameLine();
                        ImGui::Text("counter = %d", counter);

                        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                        ImGui::End();
                    }

                    // 3. Show another simple window.
                    if (show_another_window)
                    {
                        ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                        ImGui::Text("Hello from another window!");
                        if (ImGui::Button("Close Me"))
                            show_another_window = false;
                        ImGui::End();
                    }

                    // Rendering
                    ImGui::Render();
                });

            UINT backBufferIdx = swindex; //           g_pSwapChain->GetCurrentBackBufferIndex();

            deviceContext.ExecuteAsync(
                [backBufferIdx, clear_color](GAPI::Device& device)
                {
                    std::ignore = device;

                    D3D12_RESOURCE_BARRIER barrier = {};
                    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                    barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx];
                    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
                    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

                    // Render Dear ImGui graphics

                    g_CommandList->ClearRenderTargetView(g_frameBuffers[backBufferIdx]->GetDescription().renderTargetViews[0], Vector4(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w));

                    g_pd3dCommandList->ResourceBarrier(1, &barrier);
                    g_CommandList->SetFrameBuffer(g_frameBuffers[backBufferIdx]);
                    g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);

                    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_CommandList);

                    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
                    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

                    g_pd3dCommandList->ResourceBarrier(1, &barrier);
                    g_CommandList->Close();
                });

            swindex = (++swindex % NUM_BACK_BUFFERS);

            deviceContext.Submit(g_CommandQueue, g_CommandList);

            // Update and Render additional Platform Windows
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                // ImGui::UpdatePlatformWindows();
                //   ImGui::RenderPlatformWindowsDefault(nullptr, (void*)g_pd3dCommandList);
            }

            deviceContext.Present(g_pSwapChain);

            deviceContext.MoveToNextFrame(g_CommandQueue);
            //   deviceContext.WaitForGpu(g_CommandQueue);

            float current_time = (float)toolkit.GetTime();
            dt = (current_time - last_time);
            last_time = current_time;
        }

        WaitForLastSubmittedFrame();

        // Cleanup
        ImGui_ImplDX12_Shutdown();
        imguiPlatformInput.Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceD3D();

        Render::DeviceContext::Instance().Terminate();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        window = nullptr;

        return 0;
    }

    void Application::init()
    {

        //   testEvent.Register<Application>(this, &Application::bar);
    }
}