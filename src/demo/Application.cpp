#include "Application.hpp"

#include "common/OnScopeExit.hpp"
#include "common/Time.hpp"
#include "common/debug/LeakDetector.hpp"

#include "gapi/Buffer.hpp"
#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/Fence.hpp"
#include "gapi/GpuResourceViews.hpp"
#include "gapi/MemoryAllocation.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

#include "render/DeviceContext.hpp"

#include "platform/Toolkit.hpp"
#include "platform/Window.hpp"

namespace RR
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
            if (((texel.x / 4) + (texel.y / 4) + (texel.z / 4) + level) & 1)
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
        void fillTextureData(const GAPI::GpuResourceDescription& description, const GAPI::CpuResourceData::SharedPtr& textureData)
        {
            ASSERT((std::is_same<T, uint32_t>::value && description.GetFormat() == GAPI::GpuResourceFormat::RGBA8Uint) ||
                   (std::is_same<T, uint32_t>::value && description.GetFormat() == GAPI::GpuResourceFormat::BGRA8Unorm) ||
                   (std::is_same<T, Vector4>::value && description.GetFormat() == GAPI::GpuResourceFormat::RGBA16Float) ||
                   (std::is_same<T, Vector4>::value && description.GetFormat() == GAPI::GpuResourceFormat::RGBA32Float));
            ASSERT(textureData->GetFirstSubresource() == 0);

            const auto& subresourceFootprints = textureData->GetSubresourceFootprints();
            const auto dataPointer = static_cast<uint8_t*>(textureData->GetAllocation()->Map());

            for (uint32_t index = 0; index < subresourceFootprints.size(); index++)
            {
                const auto& subresourceFootprint = subresourceFootprints[index];

                for (uint32_t depth = 0; depth < subresourceFootprint.depth; depth++)
                {
                    const auto depthPointer = dataPointer + subresourceFootprint.offset +
                                              depth * subresourceFootprint.depthPitch;
                    for (uint32_t row = 0; row < subresourceFootprint.numRows; row++)
                    {
                        const auto rowPointer = depthPointer + row * subresourceFootprint.rowPitch;
                        auto columnPointer = reinterpret_cast<T*>(rowPointer);

                        for (uint32_t column = 0; column < subresourceFootprint.width; column++)
                        {
                            const auto texel = Vector3u(column, row, depth);

                            *columnPointer = checkerboardPattern<T>(texel, index);
                            columnPointer++;
                        }
                    }
                }
            }

            textureData->GetAllocation()->Unmap();
        }

        void initTextureData(const GAPI::GpuResourceDescription& description, const GAPI::CpuResourceData::SharedPtr& textureData)
        {
            switch (description.GetFormat())
            {
                case GAPI::GpuResourceFormat::RGBA8Uint:
                case GAPI::GpuResourceFormat::BGRA8Unorm:
                    fillTextureData<uint32_t>(description, textureData);
                    break;
                case GAPI::GpuResourceFormat::RGBA16Float:
                case GAPI::GpuResourceFormat::RGBA32Float:
                    fillTextureData<Vector4>(description, textureData);
                    break;
                default:
                    LOG_FATAL("Unsupported format");
            }
        }

        bool isSubresourceEqual(const GAPI::CpuResourceData::SharedPtr& lhs, uint32_t lSubresourceIndex,
                                const GAPI::CpuResourceData::SharedPtr& rhs, uint32_t rSubresourceIndex)
        {
            ASSERT(lhs);
            ASSERT(rhs);
            ASSERT(lhs != rhs);
            ASSERT(lSubresourceIndex < lhs->GetNumSubresources());
            ASSERT(rSubresourceIndex < lhs->GetNumSubresources());
            ASSERT(lhs->GetAllocation()->GetMemoryType() != GAPI::MemoryAllocationType::Upload);
            ASSERT(rhs->GetAllocation()->GetMemoryType() != GAPI::MemoryAllocationType::Upload);

            const auto ldataPointer = static_cast<uint8_t*>(lhs->GetAllocation()->Map());
            const auto rdataPointer = static_cast<uint8_t*>(rhs->GetAllocation()->Map());

            ON_SCOPE_EXIT(
                {
                    lhs->GetAllocation()->Unmap();
                    rhs->GetAllocation()->Unmap();
                });

            const auto& lfootprint = lhs->GetSubresourceFootprintAt(lSubresourceIndex);
            const auto& rfootprint = rhs->GetSubresourceFootprintAt(rSubresourceIndex);

            ASSERT(lfootprint.isComplatable(rfootprint));

            auto lrowPointer = ldataPointer + lfootprint.offset;
            auto rrowPointer = rdataPointer + rfootprint.offset;

            for (uint32_t row = 0; row < lfootprint.numRows; row++)
            {
                if (memcmp(lrowPointer, rrowPointer, lfootprint.rowSizeInBytes) != 0)
                    return false;

                lrowPointer += lfootprint.rowPitch;
                rrowPointer += rfootprint.rowPitch;
            }

            return true;
        }

        bool isResourceEqual(const GAPI::CpuResourceData::SharedPtr& lhs,
                             const GAPI::CpuResourceData::SharedPtr& rhs)
        {
            ASSERT(lhs != rhs);
            ASSERT(lhs->GetNumSubresources() == rhs->GetNumSubresources());

            const auto numSubresources = lhs->GetNumSubresources();
            for (uint32_t index = 0; index < numSubresources; index++)
                if (!isSubresourceEqual(lhs, index, rhs, index))
                    return false;

            return true;
        }

    }
    void Application::OnWindowResize(const Platform::Window&, const Vector2i& size)
    {
        GAPI::SwapChainDescription desc = swapChain_->GetDescription();
        desc.width = size.x;
        desc.height = size.y;

        auto& deviceContext = Render::DeviceContext::Instance();
        deviceContext.ResetSwapChain(swapChain_, desc);

        swindex = 0;
    }

    GAPI::Buffer::SharedPtr initBufferWithData(const char* data, GAPI::GpuResourceBindFlags bindFlags = GAPI::GpuResourceBindFlags::ShaderResource)
    {
        const auto dataBuffer = std::make_shared<DataBuffer>(strlen(data), static_cast<const void*>(data));
        const auto& description = GAPI::GpuResourceDescription::Buffer(uint32_t(dataBuffer->Size()), bindFlags);

        auto& deviceContext = Render::DeviceContext::Instance();
        return deviceContext.CreateBuffer(description, dataBuffer, GAPI::GpuResourceUsage::Default, "Source");
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

        auto& deviceContext = Render::DeviceContext::Instance();

        auto commandQueue = deviceContext.CreteCommandQueue(GAPI::CommandQueueType::Graphics, u8"Primary");
        auto commandList = deviceContext.CreateGraphicsCommandList(u8"qwew");

        const auto desc = GAPI::GpuResourceDescription::Texture2D(100, 100, GAPI::GpuResourceFormat::BGRA8Unorm, GAPI::GpuResourceBindFlags::RenderTarget, 1, 1);
        auto texture = deviceContext.CreateTexture(desc);
        //  ASSERT(commandList)
        // commandList->Close();

        const auto& description = GAPI::GpuResourceDescription::Texture3D(128, 128, 128, GAPI::GpuResourceFormat::RGBA8Uint);

        auto cpuData = deviceContext.AllocateIntermediateResourceData(description, GAPI::MemoryAllocationType::CpuReadWrite);
        auto readbackData = deviceContext.AllocateIntermediateResourceData(description, GAPI::MemoryAllocationType::Readback);

        initTextureData(description, cpuData);

        auto testTexture = deviceContext.CreateTexture(description, nullptr, GAPI::GpuResourceUsage::Default, "Test");

        GAPI::SwapChainDescription desciption = {};
        desciption.width = 100;
        desciption.height = 100;
        desciption.bufferCount = 2;
        desciption.isStereo = false;
        desciption.gpuResourceFormat = GAPI::GpuResourceFormat::BGRA8Unorm;
        desciption.window = _window;

        swapChain_ = deviceContext.CreateSwapchain(desciption, "Primary");

        auto fence = deviceContext.CreateFence("qwe");

        const auto& platform = Platform::Toolkit::Instance();

        while (!_quit)
        {
            platform.PoolEvents();
            //Platform::WindowSystem::PoolEvents();

            // deviceContext.Submit(commandQueue, commandList);

            //  renderPipeline->Collect(_scene);
            //    renderPipeline->Draw();

            //   _scene->Update();
            //device->Present();
            //    render->SwapBuffers();
            //  input->Update();

            // deviceContext.Present();

            // auto index2 = index;

            std::shared_ptr<GAPI::CpuResourceData> readbackData1;

            deviceContext.ExecuteAsync(
                [swapChain = swapChain_, index2 = swindex, commandList, texture, testTexture, cpuData, readbackData, &readbackData1](GAPI::Device& device)
                {
                    std::ignore = device;

                    auto swapChainTexture = swapChain->GetBackBufferTexture(index2);
                    //Log::Print::Info("Texture %s\n", texture->GetName());

                    {
                        const auto sourceData = "1234567890";
                        auto source = initBufferWithData(sourceData);

                        const auto destData = "QWERTYUIOP";
                        auto dest = initBufferWithData(destData);

                        auto& deviceContext = Render::DeviceContext::Instance();
                        const auto& description = GAPI::GpuResourceDescription::Buffer(uint32_t(strlen(destData)));
                        readbackData1 = deviceContext.AllocateIntermediateResourceData(description, GAPI::MemoryAllocationType::Readback);

                        commandList->CopyBufferRegion(source, 5, dest, 3, 4);
                        commandList->ReadbackGpuResource(dest, readbackData1);
                        commandList->Close();
                    }

                    {
                        /* auto& deviceContext = Render::DeviceContext::Instance();

                        const auto& sourceDescription = GAPI::GpuResourceDescription::Texture3D(256, 256, 256, GAPI::GpuResourceFormat::RGBA8Uint);
                        const auto sourceData = deviceContext.AllocateIntermediateResourceData(sourceDescription, GAPI::MemoryAllocationType::CpuReadWrite);
                        auto source = deviceContext.CreateTexture(sourceDescription, nullptr, GAPI::GpuResourceUsage::Default, "Source");

                        initTextureData(sourceDescription, sourceData);
                        commandList->UpdateGpuResource(source, sourceData);

                        const auto& destDescription = GAPI::GpuResourceDescription::Texture3D(128, 128, 128, GAPI::GpuResourceFormat::RGBA8Uint);
                        const auto destData = deviceContext.AllocateIntermediateResourceData(destDescription, GAPI::MemoryAllocationType::Upload);
                        auto dest = deviceContext.CreateTexture(destDescription, nullptr, GAPI::GpuResourceUsage::Default, "Dest");

                        initTextureData(destDescription, destData);
                        commandList->UpdateGpuResource(dest, destData);

                        commandList->CopyTextureSubresourceRegion(source, 1, Box3u(7, 42, 13, 64, 64, 64), dest, 0, Vector3u(32, 32, 32));
                        commandList->CopyTextureSubresourceRegion(source, 2, Box3u(0, 0, 0, 32, 32, 32), dest, 1, Vector3u(16, 16, 16));
                        commandList->CopyTextureSubresourceRegion(source, 0, Box3u(45, 128, 205, 16, 16, 16), dest, 2, Vector3u(0, 0, 0));

                        readbackData1 = deviceContext.AllocateIntermediateResourceData(destDescription, GAPI::MemoryAllocationType::Readback);
                        commandList->ReadbackGpuResource(dest, readbackData1);

                        commandList->Close();*/
                    }
                    /*
                    auto swapChainRtv = swapChainTexture->GetRTV();
                    auto blueRtv = texture->GetRTV();

                    commandList->ClearRenderTargetView(blueRtv, Vector4(0, 0, 1, 1));
                    commandList->ClearRenderTargetView(swapChainRtv, Vector4(static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX), 0, 0, 0));
                    // commandList->CopyTextureSubresourceRegion(texture, 0, Box3u(0, 0, 0, 50, 50, 1), swapChainTexture, 0, Vector3::ZERO);

                    commandList->UpdateGpuResource(testTexture, cpuData);
                    commandList->ReadbackGpuResource(testTexture, readbackData);
                    //commandList->Close();

                    // commandList->UpdateGpuResource(testTexture, cpuData);
                    //commandList->CopyTextureSubresourceRegion(testTexture, 0, Box3u(0, 0, 0, 50, 50, 1), swapChainTexture, 0, Vector3::ZERO);
                    //commandList->ReadbackGpuResource(testTexture, readbackData);

                    commandList->Close();*/
                });
            /*
            deviceContext.Submit(commandQueue, commandList);
            deviceContext.WaitForGpu(commandQueue);

            const auto pointer = readbackData1->GetAllocation()->Map();

            (void)pointer;
            ON_SCOPE_EXIT(
                {
                    readbackData1->GetAllocation()->Unmap();
                })*/

            deviceContext.Submit(commandQueue, commandList);
            deviceContext.WaitForGpu(commandQueue);

            const auto dataPointer = static_cast<uint8_t*>(readbackData1->GetAllocation()->Map());
            ON_SCOPE_EXIT(
                {
                    readbackData1->GetAllocation()->Unmap();
                });

            std::ignore = dataPointer;

            // const auto& footprint = readbackData->GetSubresourceFootprintAt(0);
            // REQUIRE(memcmp(dataPointer, testData, footprint.rowSizeInBytes) == 0);

            deviceContext.Present(swapChain_);
            deviceContext.MoveToNextFrame(commandQueue);

            swindex = (++swindex % swapChain_->GetDescription().bufferCount);
            frame++;

            time->Update();
        }

        cpuData = nullptr;
        readbackData = nullptr;
        texture = nullptr;
        testTexture = nullptr;
        commandQueue = nullptr;
        commandList = nullptr;

        commandQueue = nullptr;
        commandList = nullptr;
        fence = nullptr;

        //  delete renderPipeline;

        terminate();
    }

    void Application::OnClose(const Platform::Window&)
    {
        _quit = true;
    }

    void Application::init()
    {
        Platform::Window::Description windowDesc;
        windowDesc.size = Vector2i(800, 600);
        windowDesc.title = "Demo";

        auto& platform = Platform::Toolkit::Instance();
        platform.Init();

        _window = platform.CreatePlatformWindow(windowDesc);
        ASSERT(_window);
        _window->OnClose.Subscribe<Application, &Application::OnClose>(this);
        _window->OnResize.Subscribe<Application, &Application::OnWindowResize>(this);

        // Inputting::Instance()->Init();
        // Inputting::Instance()->SubscribeToWindow(_window);

        auto& deviceContext = Render::DeviceContext::Instance();
        deviceContext.Init();

        // auto& render = Rendering::Instance();
        // render->Init(_window);
    }

    void Application::terminate()
    {
        swapChain_ = nullptr;

        Render::DeviceContext::Instance().Terminate();

        //_scene->Terminate();

        _window.reset();
        _window = nullptr;

        // Inputting::Instance()->Terminate();

        // Rendering::Instance()->Terminate();
        // Platform::WindowSystem::UnSubscribe(this);
    }

    void Application::loadResouces()
    {
        // _scene = std::make_shared<Scenes::Scene_2>();
        // _scene->Init();
    }
}