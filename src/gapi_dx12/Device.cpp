#include "Device.hpp"

// TODO temporary
//#include "gapi/FencedRingBuffer.hpp"

#include "gapi/Frame.hpp"

#include "gapi_dx12/CommandContextImpl.hpp"
#include "gapi_dx12/CommandListCompiler.hpp"
#include "gapi_dx12/CommandListImpl.hpp"
#include "gapi_dx12/D3DUtils.hpp"
#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/ResourceCreator.hpp"

#include <chrono>
#include <thread>

#ifdef ENABLE_ASSERTS
#define ASSERT_IS_CREATION_THREAD ASSERT(creationThreadID_ == std::this_thread::get_id())
#define ASSERT_IS_DEVICE_INITED ASSERT(inited_)
#else
#define ASSERT_IS_CREATION_THREAD
#define ASSERT_IS_DEVICE_INITED
#endif

using namespace std::chrono_literals;

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            template <typename T>
            void ThrowIfFailed(T c) { std::ignore = c; };

            class DeviceImplementation
            {
            public:
                DeviceImplementation();

                Result Init();
                Result Reset(const PresentOptions& presentOptions);
                Result Present();

                ID3D12Device* GetDevice() const
                {
                    return d3dDevice_.get();
                }

                void WaitForGpu();

                Result InitResource(Object::ConstSharedPtrRef resource);

            private:
                bool enableDebug_ = true;

                std::thread::id creationThreadID_;
                bool inited_ = false;

                ResourceCreatorContext resourceCreatorContext_;

                ComSharedPtr<ID3D12Debug1> debugController_;
                ComSharedPtr<IDXGIFactory2> dxgiFactory_;
                ComSharedPtr<IDXGIAdapter1> dxgiAdapter_;
                ComSharedPtr<ID3D12Device> d3dDevice_;
                ComSharedPtr<IDXGISwapChain3> swapChain_;

                std::array<ComSharedPtr<ID3D12CommandQueue>, static_cast<size_t>(CommandQueueType::COUNT)> commandQueues_;
                std::array<ComSharedPtr<ID3D12Resource>, MAX_BACK_BUFFER_COUNT> renderTargets_;
                std::array<DescriptorHeap::Allocation, MAX_BACK_BUFFER_COUNT> rtvs_;

                D3D_FEATURE_LEVEL d3dFeatureLevel_ = D3D_FEATURE_LEVEL_1_0_CORE;

                uint32_t frameIndex_ = UNDEFINED_FRAME_INDEX;

                uint32_t backBufferIndex_ = 0;
                uint32_t backBufferCount_ = 0;

                std::unique_ptr<DescriptorHeapSet> descriptorHeapSet_;
                // TEMPORARY
                // std::unique_ptr<CommandContext> CommandContext_;
                std::shared_ptr<FenceImpl> fence_;
                std::array<uint64_t, GPU_FRAMES_BUFFERED> fenceValues_;
                winrt::handle fenceEvent_;
                // TEMPORARY END

                ComSharedPtr<ID3D12CommandQueue> getCommandQueue(CommandQueueType commandQueueType)
                {
                    return commandQueues_[static_cast<std::underlying_type<CommandQueueType>::type>(commandQueueType)];
                }

                Result createDevice();

                Result handleDeviceLost();

                void moveToNextFrame();
            };

            DeviceImplementation::DeviceImplementation()
                : creationThreadID_(std::this_thread::get_id())
            {
            }

            Result DeviceImplementation::Init()
            {
                ASSERT_IS_CREATION_THREAD;
                ASSERT(inited_ == false);

                auto& commandQueue = getCommandQueue(CommandQueueType::GRAPHICS);

                // TODO Take from parameters. Check by assert;
                backBufferCount_ = 2;

                D3DCallMsg(createDevice(), "CreateDevice");

                // Create the command queue.
                D3D12_COMMAND_QUEUE_DESC queueDesc = {};
                queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
                queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

                {
                    ComSharedPtr<ID3D12CommandQueue> commandQueue;

                    D3DCallMsg(d3dDevice_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(commandQueue.put())), "CreateCommandQueue");
                    D3DUtils::SetAPIName(commandQueue.get(), u8"MainCommandQueue");

                    commandQueue.as(commandQueues_[static_cast<size_t>(CommandQueueType::GRAPHICS)]);
                }

                // Create a fence for tracking GPU execution progress.
                fence_.reset(new FenceImpl());
                D3DCall(fence_->Init(d3dDevice_.get(), 1, "FrameSync"));

                //   CommandContext_.reset(new CommandContext());
                //    D3DCall(CommandContext_->Init(d3dDevice_.get(), "Main"));

                for (int i = 0; i < GPU_FRAMES_BUFFERED; i++)
                {
                    fenceValues_[i] = 0;
                }
                fenceValues_[frameIndex_] = 1;
                //_fence->SetName(L"DeviceResources");

                fenceEvent_.attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
                if (!bool { fenceEvent_ })
                {
                    LOG_ERROR("Failure create fence Event");
                    return Result::Fail;
                }

                descriptorHeapSet_ = std::make_unique<DescriptorHeapSet>();
                D3DCall(descriptorHeapSet_->Init(d3dDevice_.get()));

                resourceCreatorContext_.device = d3dDevice_.get();
                resourceCreatorContext_.descriptorHeapSet = descriptorHeapSet_.get();

                inited_ = true;
                return Result::Ok;
            }

            void DeviceImplementation::WaitForGpu()
            {
                ASSERT_IS_DEVICE_INITED;
            }

            Result DeviceImplementation::InitResource(Object::ConstSharedPtrRef resource)
            {
                return ResourceCreator::InitResource(resourceCreatorContext_, resource);
            }

            // These resources need to be recreated every time the window size is changed.
            Result DeviceImplementation::Reset(const PresentOptions& presentOptions)
            {
                ASSERT_IS_CREATION_THREAD;
                ASSERT_IS_DEVICE_INITED;
                ASSERT(presentOptions.windowHandle);
                ASSERT(presentOptions.isStereo == false);

                if (!backBufferCount_)
                    backBufferCount_ = presentOptions.bufferCount;

                ASSERT_MSG(presentOptions.bufferCount == backBufferCount_, "Changing backbuffer count should work, but this is untested")

                const HWND windowHandle = presentOptions.windowHandle;

                // Wait until all previous GPU work is complete.
                WaitForGpu();

                // Release resources that are tied to the swap chain and update fence values.
                for (uint32_t n = 0; n < backBufferCount_; n++)
                {
                    renderTargets_[n] = nullptr;
                    // m_fenceValues[n] = m_fenceValues[m_frameIndex];
                }

                // If the swap chain already exists, resize it, otherwise create one.
                if (swapChain_)
                {
                    DXGI_SWAP_CHAIN_DESC1 currentSwapChainDesc;

                    D3DCallMsg(swapChain_->GetDesc1(&currentSwapChainDesc), "GetDesc1");

                    const auto& targetSwapChainDesc = D3DUtils::GetDXGISwapChainDesc1(presentOptions, DXGI_SWAP_EFFECT_FLIP_DISCARD);
                    const auto swapChainCompatable = D3DUtils::SwapChainDesc1MatchesForReset(currentSwapChainDesc, targetSwapChainDesc);

                    if (!swapChainCompatable)
                    {
                        LOG_ERROR("SwapChains incompatible");
                        return Result::Fail;
                    }

                    // If the swap chain already exists, resize it.
                    HRESULT hr = swapChain_->ResizeBuffers(
                        targetSwapChainDesc.BufferCount,
                        targetSwapChainDesc.Width,
                        targetSwapChainDesc.Height,
                        targetSwapChainDesc.Format,
                        targetSwapChainDesc.Flags);

                    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
                    {
                        LOG_ERROR("Device Lost on ResizeBuffers: Reason code 0x%08X\n",
                            static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? d3dDevice_->GetDeviceRemovedReason() : hr))

                        // If the device was removed for any reason, a new device and swap chain will need to be created.
                        // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method
                        // and correctly set up the new device.
                        return handleDeviceLost();
                    }
                    else
                        D3DCallMsg(hr, "ResizeBuffers");
                }
                else
                {
                    const auto& graphicsCommandQueue = getCommandQueue(CommandQueueType::GRAPHICS);
                    const auto& targetSwapChainDesc = D3DUtils::GetDXGISwapChainDesc1(presentOptions, DXGI_SWAP_EFFECT_FLIP_DISCARD);

                    ComSharedPtr<IDXGISwapChain1> swapChain2;
                    // Create a swap chain for the window.
                    D3DCallMsg(dxgiFactory_->CreateSwapChainForHwnd(
                                   graphicsCommandQueue.get(),
                                   presentOptions.windowHandle,
                                   &targetSwapChainDesc,
                                   nullptr,
                                   nullptr,
                                   swapChain2.put()),
                        "CreateSwapChainForHwnd");

                    swapChain2.as(swapChain_);
                }

                // Update backbuffers.
                DXGI_SWAP_CHAIN_DESC1 currentSwapChainDesc;
                D3DCallMsg(swapChain_->GetDesc1(&currentSwapChainDesc), "GetDesc1");

                for (uint32_t index = 0; index < backBufferCount_; index++)
                {
                    ThrowIfFailed(swapChain_->GetBuffer(index, IID_PPV_ARGS(renderTargets_[index].put())));
                    D3DUtils::SetAPIName(renderTargets_[index].get(), "BackBuffer", index);

                    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                    rtvDesc.Format = currentSwapChainDesc.Format;
                    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                }

                backBufferIndex_ = 0;

                // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
                //   if (ResultU::Failure(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER)))
                // {
                // }*/

                return Result::Ok;
            }

            Result DeviceImplementation::Present()
            {
                ASSERT_IS_CREATION_THREAD;
                ASSERT_IS_DEVICE_INITED;

                const auto& commandQueue = getCommandQueue(CommandQueueType::GRAPHICS);

                const UINT64 currentFenceValue = fenceValues_[frameIndex_];

                {
                    // Transition the render target to the state that allows it to be presented to the display.
                    // D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_backBufferIndex].Get(), beforeState, D3D12_RESOURCE_STATE_PRESENT);
                    //commandList->ResourceBarrier(1, &barrier);
                }

                D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets_[backBufferIndex_].get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

                for (int i = 0; i < 100000; i++)
                {
                    Vector4 color(std::rand() / static_cast<float>(RAND_MAX),
                        std::rand() / static_cast<float>(RAND_MAX),
                        std::rand() / static_cast<float>(RAND_MAX), 1);

                    //  CommandContext_->ClearRenderTargetView(rtvs_[backBufferIndex_], color);
                }

                //HRESULT hr;
                /*if (m_options & c_AllowTearing)
                {
                    // Recommended to always use tearing if supported when using a sync interval of 0.
                    // Note this will fail if in true 'fullscreen' mode.
                    hr = m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
                }
                else
                {*/
                // The first argument instructs DXGI to block until VSync, putting the application
                // to sleep until the next VSync. This ensures we don't waste any cycles rendering
                // frames that will never be displayed to the screen.
                //  hr = swapChain->Present(0, 0);
                //  }

                // The first argument instructs DXGI to block until VSync, putting the application
                // to sleep until the next VSync. This ensures we don't waste any cycles rendering
                // frames that will never be displayed to the screen.
                DXGI_PRESENT_PARAMETERS parameters
                    = {};
                //  std::this_thread::sleep_for(10ms);
                HRESULT hr = swapChain_->Present1(0, 0, &parameters);

                // If the device was reset we must completely reinitialize the renderer.
                if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
                {
                    Log::Print::Warning("Device Lost on Present: Reason code 0x%08X\n", static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? d3dDevice_->GetDeviceRemovedReason() : hr));

                    // Todo error check
                    handleDeviceLost();
                }
                else
                {
                    D3DCallMsg(hr, "Present1");

                    moveToNextFrame();

                    if (!dxgiFactory_->IsCurrent())
                    {
                        LOG_ERROR("Dxgi is not current");
                        return Result::Fail;

                        // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
                        //ThrowIfFailed(CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(m_dxgiFactory.ReleaseAndGetAddressOf())));
                    }
                }

                return Result::Ok;
            }

            Result DeviceImplementation::handleDeviceLost()
            {
                // Todo implement properly Device lost event processing
                Log::Print::Fatal("Device was lost.\n");
                return Result::Ok;
            }

            Result DeviceImplementation::createDevice()
            {
                ASSERT_IS_CREATION_THREAD;

                UINT dxgiFactoryFlags = 0;
                // Enable the debug layer (requires the Graphics Tools "optional feature").
                // NOTE: Enabling the debug layer after device creation will invalidate the active device.
                if (enableDebug_)
                {
                    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController_.put()))))
                    {
                        debugController_->EnableDebugLayer();
                        debugController_->SetEnableGPUBasedValidation(true);
                        debugController_->SetEnableSynchronizedCommandQueueValidation(true);
                    }
                    else
                    {
                        LOG_WARNING("WARNING: Direct3D Debug Device is not available\n");
                    }

                    ComSharedPtr<IDXGIInfoQueue> dxgiInfoQueue;
                    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.put()))))
                    {
                        dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

                        dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
                        dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
                    }
                }

                D3DCallMsg(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(dxgiFactory_.put())), "CreateDXGIFactory2");

                D3D_FEATURE_LEVEL minimumFeatureLevel = D3D_FEATURE_LEVEL_11_0;

                D3DCallMsg(D3DUtils::GetAdapter(dxgiFactory_, minimumFeatureLevel, dxgiAdapter_), "GetAdapter");

                // Create the DX12 API device object.
                D3DCallMsg(D3D12CreateDevice(dxgiAdapter_.get(), minimumFeatureLevel, IID_PPV_ARGS(d3dDevice_.put())), "D3D12CreateDevice");

                D3DUtils::SetAPIName(d3dDevice_.get(), "Main");

                if (enableDebug_)
                {
                    // Configure debug device (if active).
                    ComSharedPtr<ID3D12InfoQueue> d3dInfoQueue;

                    Result result;
                    if (result = Result(d3dDevice_->QueryInterface(IID_PPV_ARGS(d3dInfoQueue.put()))))
                    {
                        d3dInfoQueue->ClearRetrievalFilter();
                        d3dInfoQueue->ClearStorageFilter();

                        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
                        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
                    }
                    else
                    {
                        LOG_ERROR("Unable to get ID3D12InfoQueue with HRESULT of 0x%08X", result);
                    }
                }

                // Determine maximum supported feature level for this device
                static const D3D_FEATURE_LEVEL s_featureLevels[] = {
                    D3D_FEATURE_LEVEL_12_1,
                    D3D_FEATURE_LEVEL_12_0,
                    D3D_FEATURE_LEVEL_11_1,
                    D3D_FEATURE_LEVEL_11_0,
                };

                D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels = {
                    _countof(s_featureLevels), s_featureLevels, D3D_FEATURE_LEVEL_11_0
                };

                if (SUCCEEDED(d3dDevice_->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels))))
                {
                    d3dFeatureLevel_ = featLevels.MaxSupportedFeatureLevel;
                }
                else
                {
                    d3dFeatureLevel_ = minimumFeatureLevel;
                }

                return Result::Ok;
            }

            void DeviceImplementation::moveToNextFrame()
            {
                ASSERT_IS_CREATION_THREAD;
                ASSERT_IS_DEVICE_INITED;

                const auto& commandQueue = getCommandQueue(CommandQueueType::GRAPHICS);

                // Schedule a Signal command in the queue.
                const UINT64 currentFenceValue = fenceValues_[frameIndex_];
                // ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

                fence_->Signal(commandQueue.get(), currentFenceValue);

                // Update the back buffer index.
                backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
                frameIndex_ = (frameIndex_++ % GPU_FRAMES_BUFFERED);

                // If the next frame is not ready to be rendered yet, wait until it is ready.
                if (fence_->GetGpuValue() < fenceValues_[frameIndex_])
                {
                    fence_->SetEventOnCompletion(fenceValues_[frameIndex_], fenceEvent_.get());
                    //   ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_backBufferIndex], m_fenceEvent.Get()));
                    WaitForSingleObjectEx(fenceEvent_.get(), INFINITE, FALSE);
                }

                // Set the fence value for the next frame.
                fenceValues_[frameIndex_] = currentFenceValue + 1;
            }

            Device::Device()
                : _impl(new DeviceImplementation())
            {
            }

            Device::~Device() { }

            Result Device::Init()
            {
                return _impl->Init();
            }

            Result Device::Reset(const PresentOptions& presentOptions)
            {
                return _impl->Reset(presentOptions);
            }

            Result Device::Present()
            {
                return _impl->Present();
            }

            void Device::WaitForGpu()
            {
                return _impl->WaitForGpu();
            }

            uint64_t Device::GetGpuFenceValue(Fence::ConstSharedPtrRef fence) const
            {
                return 0;
            }

            Result Device::InitResource(Object::ConstSharedPtrRef resource)
            {
                return _impl->InitResource(resource);
            }
        }
    }
}