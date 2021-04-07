#include "Application.hpp"

#include "common/Time.hpp"
#include "common/debug/LeakDetector.hpp"

#include "resource_manager/ResourceManager.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/Fence.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/MemoryAllocation.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

#include "render/RenderContext.hpp"

#include "windowing/Window.hpp"
#include "windowing/WindowSystem.hpp"

#include "scenes/Scene_1.hpp"
#include "scenes/Scene_2.hpp"

namespace OpenDemo
{
    using namespace Common;

    static uint32_t swindex = 0;
    static uint32_t frame = 0;

    namespace
    {
        template <typename T>
        T texelZeroFill(Vector3u texel, uint32_t level)
        {
            return T(0);
        }

        template <typename T>
        T checkerboardPattern(Vector3u texel, uint32_t level);

        template <>
        Vector4 checkerboardPattern(Vector3u texel, uint32_t level)
        {
            if (((texel.x + texel.y + texel.z) / 8 + level) & 1)
            {
                return Vector4(0.5f, 0.5f, 0.5f, 0.5f);
            }

            std::array<Vector4, 8> colors = {
                Vector4(0, 0, 1, 1),
                Vector4(0, 1, 0, 1),
                Vector4(1, 0, 0, 1),
                Vector4(0, 1, 1, 1),
                Vector4(1, 0, 1, 1),
                Vector4(1, 1, 0, 1),
                Vector4(1, 1, 1, 1),
                Vector4(0.25, 0.25, 0.25, 1),
            };

            return colors[std::min(level, 7u)];
        }

        template <>
        uint32_t checkerboardPattern(Vector3u texel, uint32_t level)
        {
            const auto& value = checkerboardPattern<Vector4>(texel, level);
            // TODO olor class
            // RGBA format
            return static_cast<uint32_t>(value.x * 255.0f) << 24 |
                   static_cast<uint32_t>(value.y * 255.0f) << 16 |
                   static_cast<uint32_t>(value.z * 255.0f) << 8 |
                   static_cast<uint32_t>(value.w * 255.0f);
        }

        template <typename T>
        void fillTextureData(const GAPI::TextureDescription& description, const GAPI::IntermediateMemory::SharedPtr& textureData, const GAPI::IntermediateMemory::SharedPtr& data)
        {
            ASSERT((std::is_same<T, uint32_t>::value && description.GetFormat() == GAPI::GpuResourceFormat::RGBA8Uint) ||
                   (std::is_same<T, Vector4>::value && description.GetFormat() == GAPI::GpuResourceFormat::RGBA16Float) ||
                   (std::is_same<T, Vector4>::value && description.GetFormat() == GAPI::GpuResourceFormat::RGBA32Float));

            const auto& subresourceFootprints = data->GetSubresourceFootprints();
            const auto dataPointer = static_cast<uint8_t*>(textureData->GetAllocation()->Map());

            for (uint32_t index = 0; index < subresourceFootprints.size(); index++)
            {
                const auto& subresourceFootprint = subresourceFootprints[index];

                auto depthPointer = dataPointer + subresourceFootprint.offset;
                for (uint32_t depth = 0; depth < subresourceFootprint.numRows; depth++)
                {
                    auto rowPointer = depthPointer;
                    for (uint32_t row = 0; row < subresourceFootprint.numRows; row++)
                    {
                        auto columnPointer = reinterpret_cast<T*>(rowPointer);
                        for (uint32_t column = 0; column < description.GetWidth(); column++)
                        {
                            const auto texel = Vector3u(column, row, depth);

                            *columnPointer = checkerboardPattern<T>(texel, index);
                            columnPointer++;
                        }

                        rowPointer += subresourceFootprint.rowPitch;
                    }
                    depthPointer += subresourceFootprint.depthPitch;
                }
            }

            textureData->GetAllocation()->Unmap();
        }

        void initTextureData(const GAPI::TextureDescription& description, const GAPI::IntermediateMemory::SharedPtr& data)
        {
            auto& renderContext = Render::RenderContext::Instance();
            const auto textureData = renderContext.AllocateIntermediateTextureData(description, GAPI::MemoryAllocationType::Upload);

            switch (description.GetFormat())
            {
            case GAPI::GpuResourceFormat::RGBA8Uint:
                fillTextureData<uint32_t>(description, textureData, data);
                break;
            case GAPI::GpuResourceFormat::RGBA16Float:
            case GAPI::GpuResourceFormat::RGBA32Float:
                fillTextureData<Vector4>(description, textureData, data);
                break;
            default:
                LOG_FATAL("Unsupported format");
            }
        }
    }
    void Application::OnWindowResize(uint32_t width, uint32_t height)
    {
        GAPI::SwapChainDescription desc = swapChain_->GetDescription();
        desc.width = width;
        desc.height = height;

        auto& renderContext = Render::RenderContext::Instance();
        renderContext.ResetSwapChain(swapChain_, desc);

        swindex = 0;
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

        const auto desc = GAPI::TextureDescription::Create2D(100, 100, GAPI::GpuResourceFormat::BGRA8Unorm, GAPI::GpuResourceBindFlags::RenderTarget, 1, 1);
        auto texture = renderContext.CreateTexture(desc);
        //  ASSERT(commandList)
        // commandList->Close();

        const auto& description = GAPI::TextureDescription::Create3D(128, 128, 128, GAPI::GpuResourceFormat::RGBA8Uint);

        const auto cpuData = renderContext.AllocateIntermediateTextureData(description, GAPI::MemoryAllocationType::CpuReadWrite);
        const auto readbackData = renderContext.AllocateIntermediateTextureData(description, GAPI::MemoryAllocationType::Readback);

        initTextureData(description, cpuData);

        auto testTexture = renderContext.CreateTexture(description, GAPI::GpuResourceCpuAccess::None, "Test");

        GAPI::SwapChainDescription desciption = {};
        desciption.width = 100;
        desciption.height = 100;
        desciption.bufferCount = 2;
        desciption.isStereo = false;
        desciption.gpuResourceFormat = GAPI::GpuResourceFormat::BGRA8Unorm;
        desciption.window = _window;

        swapChain_ = renderContext.CreateSwapchain(desciption, "Primary");

        auto fence = renderContext.CreateFence("qwe");

        const auto& windowSystem = Windowing::WindowSystem::Instance();

        while (!_quit)
        {
            windowSystem.PoolEvents();
            //Windowing::WindowSystem::PoolEvents();

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
                [swapChain = swapChain_, index2 = swindex, commandList, texture, testTexture, cpuData, readbackData](GAPI::Device& device) {
                    std::ignore = device;

                    auto swapChainTexture = swapChain->GetTexture(index2);
                    //Log::Print::Info("Texture %s\n", texture->GetName());

                    auto swapChainRtv = swapChainTexture->GetRTV();
                    auto blueRtv = texture->GetRTV();

                    commandList->ClearRenderTargetView(blueRtv, Vector4(0, 0, 1, 1));
                    commandList->ClearRenderTargetView(swapChainRtv, Vector4(static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX), 0, 0, 0));
                    commandList->CopyTextureSubresourceRegion(texture, 0, Box3u(0, 0, 0, 50, 50, 1), swapChainTexture, 0, Vector3::ZERO);

                    commandList->UpdateTexture(testTexture, cpuData);
                    commandList->ReadbackTexture(testTexture, readbackData);

                    commandList->Close();
                });

            renderContext.Submit(commandQueue, commandList);

            renderContext.Present(swapChain_);
            renderContext.MoveToNextFrame(commandQueue);

            swindex = (++swindex % swapChain_->GetDescription().bufferCount);
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

    void Application::OnClose()
    {
        _quit = true;
    }

    void Application::init()
    {
        Windowing::Window::Description windowDesc;
        windowDesc.Width = 800;
        windowDesc.Height = 600;
        windowDesc.Title = "OpenDemo";

        auto& windowSystem = Windowing::WindowSystem::Instance();
        windowSystem.Init();

        _window = windowSystem.Create(this, windowDesc);
        ASSERT(_window);

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
        // Windowing::WindowSystem::UnSubscribe(this);
    }

    void Application::loadResouces()
    {
        // _scene = std::make_shared<Scenes::Scene_2>();
        // _scene->Init();
    }
}