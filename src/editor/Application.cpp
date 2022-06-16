#include "Application.hpp"

#include <Windows.h>

#include "imgui_impl/ImguiPlatformImpl.hpp"
#include "imgui_impl/ImguiRenderImpl.hpp"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <imgui.h>
#include <tchar.h>

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/Texture.hpp"
#include "render/DeviceContext.hpp"

#include "platform/Toolkit.hpp"
#include "platform/Window.hpp"

namespace RR
{
    namespace
    {
        struct FrameContext
        {
            ID3D12CommandAllocator* CommandAllocator;
        };

        // Forward declarations of helper functions
        bool CreateDeviceD3D(const Platform::Window::SharedPtr&);
        void CleanupDeviceD3D();
        void CreateRenderTarget();
        void CleanupRenderTarget();
        void WaitForLastSubmittedFrame();
        FrameContext* WaitForNextFrameResources();

        // Data
        static int const NUM_FRAMES_IN_FLIGHT = 3;
        static FrameContext g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
        static UINT g_frameIndex = 0;

        static uint32_t swindex = 0;
        static int const NUM_BACK_BUFFERS = 3;
        static ID3D12Device* g_pd3dDevice = NULL;
        static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = NULL;
        static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
        static ID3D12CommandQueue* g_pd3dCommandQueue = NULL;
        static GAPI::CommandQueue::SharedPtr g_CommandQueue = NULL;
        static GAPI::GraphicsCommandList::SharedPtr g_CommandList = NULL;
        static ID3D12GraphicsCommandList* g_pd3dCommandList = NULL;
        static GAPI::SwapChain::SharedPtr g_pSwapChain;
        static HANDLE g_hSwapChainWaitableObject = NULL;
        static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
        static D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

        // Helper functions
        bool CreateDeviceD3D(const Platform::Window::SharedPtr& window)
        {
            // Setup swap chain
            DXGI_SWAP_CHAIN_DESC1 sd;
            {
                ZeroMemory(&sd, sizeof(sd));
                sd.BufferCount = NUM_BACK_BUFFERS;
                sd.Width = 0;
                sd.Height = 0;
                sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
                sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                sd.SampleDesc.Count = 1;
                sd.SampleDesc.Quality = 0;
                sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
                sd.Scaling = DXGI_SCALING_STRETCH;
                sd.Stereo = FALSE;
            }

            auto& deviceContext = Render::DeviceContext::Instance();
            deviceContext.Init();
            deviceContext.ExecuteAwait([](GAPI::Device& device)
                                       { g_pd3dDevice = std::any_cast<ID3D12Device*>(device.GetRawDevice()); });

            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                desc.NumDescriptors = NUM_BACK_BUFFERS;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                desc.NodeMask = 1;
                if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
                    return false;

                SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
                for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
                {
                    g_mainRenderTargetDescriptor[i] = rtvHandle;
                    rtvHandle.ptr += rtvDescriptorSize;
                }
            }

            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                desc.NumDescriptors = 1;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
                    return false;
            }
            g_CommandQueue = deviceContext.CreteCommandQueue(GAPI::CommandQueueType::Graphics, "Primary");
            g_pd3dCommandQueue = std::any_cast<ID3D12CommandQueue*>(g_CommandQueue->GetNativeHandle());

            for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
                if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator)) != S_OK)
                    return false;

            g_CommandList = deviceContext.CreateGraphicsCommandList("Primary");
            g_pd3dCommandList = std::any_cast<ID3D12GraphicsCommandList*>(g_CommandList->GetNativeHandle());
            //  g_pd3dCommandList->Close();

            GAPI::SwapChainDescription desciption = {};
            desciption.width = window->GetSize().x;
            desciption.height = window->GetSize().y;
            desciption.bufferCount = NUM_BACK_BUFFERS;
            desciption.isStereo = false;
            desciption.gpuResourceFormat = GAPI::GpuResourceFormat::RGBA8Unorm;
            desciption.window = window;

            g_pSwapChain = deviceContext.CreateSwapchain(desciption, "Primary");
            g_hSwapChainWaitableObject = std::any_cast<HANDLE>(g_pSwapChain->GetWaitableObject());

            CreateRenderTarget();
            return true;
        }

        void CleanupDeviceD3D()
        {
            CleanupRenderTarget();

            if (g_hSwapChainWaitableObject != NULL)
            {
                CloseHandle(g_hSwapChainWaitableObject);
            }
            for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
                if (g_frameContext[i].CommandAllocator)
                {
                    g_frameContext[i].CommandAllocator->Release();
                    g_frameContext[i].CommandAllocator = NULL;
                }
            if (g_pd3dCommandQueue)
            {
                g_pd3dCommandQueue->Release();
                g_pd3dCommandQueue = NULL;
            }
            if (g_pd3dCommandList)
            {
                g_pd3dCommandList = NULL;
            }
            if (g_pSwapChain)
            {
                g_pSwapChain = NULL;
            }
            if (g_pd3dRtvDescHeap)
            {
                g_pd3dRtvDescHeap->Release();
                g_pd3dRtvDescHeap = NULL;
            }
            if (g_pd3dSrvDescHeap)
            {
                g_pd3dSrvDescHeap->Release();
                g_pd3dSrvDescHeap = NULL;
            }
        }

        void CreateRenderTarget()
        {
            for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
            {
                ID3D12Resource* pBackBuffer = NULL;
                pBackBuffer = std::any_cast<ID3D12Resource*>(g_pSwapChain->GetBackBufferTexture(i)->GetRawHandle());
                g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, g_mainRenderTargetDescriptor[i]);
                g_mainRenderTargetResource[i] = pBackBuffer;
            }
        }

        void CleanupRenderTarget()
        {
            WaitForLastSubmittedFrame();

            for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
                if (g_mainRenderTargetResource[i])
                {
                    g_mainRenderTargetResource[i]->Release();
                    g_mainRenderTargetResource[i] = NULL;
                }
        }

        void WaitForLastSubmittedFrame()
        {
            Render::DeviceContext::Instance().WaitForGpu(g_CommandQueue);
        }

        FrameContext* WaitForNextFrameResources()
        {
            UINT nextFrameIndex = g_frameIndex + 1;
            g_frameIndex = nextFrameIndex;

            HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, NULL };
            DWORD numWaitableObjects = 1;

            FrameContext* frameCtx = &g_frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];

            WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

            return frameCtx;
        }
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
        ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,
                            DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
                            g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
                            g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
        //IM_ASSERT(font != NULL);

        // Our state
        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        // Main loop
        auto& deviceContext = Render::DeviceContext::Instance();
        const auto& toolkit = Platform::Toolkit::Instance();
        bool done = false;
        float dt = 0;
        float last_time = (float)toolkit.GetTime();
        while (!done)
        {
            // Poll and handle messages (inputs, window resize, etc.)
            // See the WndProc() function below for our to dispatch events to the Win32 backend.
            MSG msg;
            while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                if (msg.message == WM_QUIT)
                    done = true;
            }
            if (done)
                break;

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

            FrameContext* frameCtx = WaitForNextFrameResources();
            UINT backBufferIdx = swindex; //           g_pSwapChain->GetCurrentBackBufferIndex();
            frameCtx->CommandAllocator->Reset();

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
                    //   g_pd3dCommandList->Reset(frameCtx->CommandAllocator, NULL);
                    g_pd3dCommandList->ResourceBarrier(1, &barrier);

                    // Render Dear ImGui graphics
                    const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
                    g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, NULL);
                    g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, NULL);
                    g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
                    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);
                    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
                    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
                    g_pd3dCommandList->ResourceBarrier(1, &barrier);
                    g_CommandList->Close();
                });

            swindex = (++swindex % NUM_BACK_BUFFERS);

            deviceContext.Submit(g_CommandQueue, g_CommandList);
            //    g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&g_pd3dCommandList);

            // Update and Render additional Platform Windows
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                // ImGui::UpdatePlatformWindows();
                //   ImGui::RenderPlatformWindowsDefault(NULL, (void*)g_pd3dCommandList);
            }

            deviceContext.Present(g_pSwapChain);
            deviceContext.MoveToNextFrame(g_CommandQueue);

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
        window = nullptr;

        return 0;
    }

    void Application::init()
    {

        //   testEvent.Register<Application>(this, &Application::bar);
    }
}