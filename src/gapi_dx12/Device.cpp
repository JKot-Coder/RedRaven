#include "Device.hpp"

// TODO temporary
//#include "gapi/FencedRingBuffer.hpp"

#include "gapi_dx12/CommandListImpl.hpp"
#include "gapi_dx12/FenceImpl.hpp"

#include "gapi_dx12/D3DUtils.hpp"
#include "gapi_dx12/d3dx12.h"

#include <vector>

#include <chrono>
#include <thread>

#ifdef ENABLE_ASSERTS
#define ASSERT_IS_CREATION_THREAD ASSERT(_creationThreadID == std::this_thread::get_id())
#else
#define ASSERT_IS_CREATION_THREAD
#endif

using namespace std::chrono_literals;

namespace OpenDemo
{
    namespace Render
    {
        namespace Device
        {
            namespace DX12
            {
                template <typename T>
                void ThrowIfFailed(T c) { std::ignore = c; };

                class DeviceImplementation
                {
                public:
                    DeviceImplementation();

                    GAPIStatus Init();
                    GAPIStatus Reset(const PresentOptions& presentOptions);
                    GAPIStatus Present();

                    void WaitForGpu();

                private:
                    bool _enableDebug = true;

                    std::thread::id _creationThreadID;

                    ComSharedPtr<ID3D12Debug1> _debugController;
                    ComSharedPtr<IDXGIFactory2> _dxgiFactory;
                    ComSharedPtr<IDXGIAdapter1> _dxgiAdapter;
                    ComSharedPtr<ID3D12Device> _d3dDevice;
                    ComSharedPtr<IDXGISwapChain3> _swapChain;

                    std::array<ComSharedPtr<ID3D12CommandQueue>, static_cast<size_t>(CommandQueueType::COUNT)> _commandQueues;
                    std::array<ComSharedPtr<ID3D12Resource>, MAX_BACK_BUFFER_COUNT> _renderTargets;

                    D3D_FEATURE_LEVEL _d3dFeatureLevel = D3D_FEATURE_LEVEL_1_0_CORE;

                    uint32_t _frameIndex = 0;

                    uint32_t _backBufferIndex = 0;
                    uint32_t _backBufferCount = 0;

                    // TEMPORARY
                    ComSharedPtr<ID3D12DescriptorHeap> _rtvDescriptorHeap;
                    std::unique_ptr<CommandListImpl> _commandList;
                    std::shared_ptr<FenceImpl> _fence;
                    uint64_t _fenceValues[GPU_FRAMES_BUFFERED];
                    winrt::handle _fenceEvent;
                    uint32_t _rtvDescriptorSize;
                    // TEMPORARY END

                    CD3DX12_CPU_DESCRIPTOR_HANDLE getRenderTargetView(uint32_t backBufferIndex)
                    {
                        return CD3DX12_CPU_DESCRIPTOR_HANDLE(
                            _rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                            static_cast<INT>(backBufferIndex), _rtvDescriptorSize);
                    }

                    ComSharedPtr<ID3D12CommandQueue> getCommandQueue(CommandQueueType commandQueueType)
                    {
                        return _commandQueues[static_cast<std::underlying_type<CommandQueueType>::type>(commandQueueType)];
                    }

                    GAPIStatus createDevice();

                    GAPIStatus handleDeviceLost();

                    void moveToNextFrame();
                };

                DeviceImplementation::DeviceImplementation()
                    : _creationThreadID(std::this_thread::get_id())
                {
                }

                GAPIStatus DeviceImplementation::Init()
                {
                    ASSERT_IS_CREATION_THREAD;

                    auto& commandQueue = getCommandQueue(CommandQueueType::GRAPHICS);

                    // TODO Take from parameters. Check by assert;
                    _backBufferCount = 2;

                    GAPIStatus result = GAPIStatus::OK;

                    if (GAPIStatusU::Failure(result = createDevice()))
                    {
                        LOG_ERROR("Failed CreateDevice");
                        return result;
                    }

                    // Create the command queue.
                    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
                    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
                    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

                    {
                        ComSharedPtr<ID3D12CommandQueue> commandQueue;
                        if (GAPIStatusU::Failure(result = GAPIStatus(_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(commandQueue.put())))))
                        {
                            LOG_ERROR("Failure create CommandQueue with HRESULT of 0x%08X", result);
                            return result;
                        }
                        commandQueue->SetName(L"MainCommanQueue");

                        commandQueue.as(_commandQueues[static_cast<size_t>(CommandQueueType::GRAPHICS)]);
                    }

                    // Create descriptor heaps for render target views and depth stencil views.
                    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
                    rtvDescriptorHeapDesc.NumDescriptors = MAX_BACK_BUFFER_COUNT;
                    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

                    if (GAPIStatusU::Failure(result = GAPIStatus(_d3dDevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(_rtvDescriptorHeap.put())))))
                    {
                        LOG_ERROR("Failure create DescriptorHeap with HRESULT of 0x%08X", result);
                        return result;
                    }

                    _rtvDescriptorHeap->SetName(L"DescriptorHead");

                    _rtvDescriptorSize = _d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
                    /*
                    if (m_depthBufferFormat != DXGI_FORMAT_UNKNOWN)
                    {
                        D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
                        dsvDescriptorHeapDesc.NumDescriptors = 1;
                        dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

                        ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(m_dsvDescriptorHeap.ReleaseAndGetAddressOf())));

                        m_dsvDescriptorHeap->SetName(L"DeviceResources");
                    }*/

                    // Create a fence for tracking GPU execution progress.
                    _fence.reset(new FenceImpl());
                    if (GAPIStatusU::Failure(result = _fence->Init(_d3dDevice.get(), 1)))
                    {
                        return result;
                    }

                    _commandList.reset(new CommandListImpl(D3D12_COMMAND_LIST_TYPE_DIRECT));
                    if (GAPIStatusU::Failure(_commandList->Init(_d3dDevice.get(), _fence)))
                    {
                        return result;
                    }

                    for (int i = 0; i < GPU_FRAMES_BUFFERED; i++)
                    {
                        _fenceValues[i] = 0;
                    }
                    _fenceValues[_frameIndex] = 1;
                    //_fence->SetName(L"DeviceResources");

                    _fenceEvent.attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
                    if (!bool { _fenceEvent })
                    {
                        LOG_ERROR("Failure create fence Event");
                        return GAPIStatus::FAIL;
                    }
                }

                void DeviceImplementation::WaitForGpu()
                {
                }

                // These resources need to be recreated every time the window size is changed.
                GAPIStatus DeviceImplementation::Reset(const PresentOptions& presentOptions)
                {
                    ASSERT_IS_CREATION_THREAD;
                    ASSERT(presentOptions.windowHandle);
                    ASSERT(presentOptions.isStereo == false);

                    if (!_backBufferCount)
                        _backBufferCount = presentOptions.bufferCount;

                    ASSERT_MSG(presentOptions.bufferCount == _backBufferCount, "Changing backbuffer count should work, but this is untested")

                    const HWND windowHandle = presentOptions.windowHandle;

                    // Wait until all previous GPU work is complete.
                    WaitForGpu();

                    // Release resources that are tied to the swap chain and update fence values.
                    for (int n = 0; n < _backBufferCount; n++)
                    {
                        _renderTargets[n] = nullptr;
                        // m_fenceValues[n] = m_fenceValues[m_frameIndex];
                    }

                    GAPIStatus result = GAPIStatus::OK;

                    // If the swap chain already exists, resize it, otherwise create one.
                    if (_swapChain)
                    {
                        DXGI_SWAP_CHAIN_DESC1 currentSwapChainDesc;
                        if (GAPIStatusU::Failure(result = GAPIStatus(_swapChain->GetDesc1(&currentSwapChainDesc))))
                        {
                            LOG_ERROR("Failure get swapChain Desc");
                            return result;
                        }

                        const auto& targetSwapChainDesc = D3DUtils::GetDXGISwapChainDesc1(presentOptions, DXGI_SWAP_EFFECT_FLIP_DISCARD);
                        const auto swapChainCompatable = D3DUtils::SwapChainDesc1MatchesForReset(currentSwapChainDesc, targetSwapChainDesc);

                        if (!swapChainCompatable)
                        {
                            LOG_ERROR("SwapChains incompatible");
                            return GAPIStatus::FAIL;
                        }

                        // If the swap chain already exists, resize it.
                        HRESULT hr = _swapChain->ResizeBuffers(
                            targetSwapChainDesc.BufferCount,
                            targetSwapChainDesc.Width,
                            targetSwapChainDesc.Height,
                            targetSwapChainDesc.Format,
                            targetSwapChainDesc.Flags);

                        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
                        {
                            LOG_ERROR("Device Lost on ResizeBuffers: Reason code 0x%08X\n",
                                static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? _d3dDevice->GetDeviceRemovedReason() : hr))

                            // If the device was removed for any reason, a new device and swap chain will need to be created.
                            // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method
                            // and correctly set up the new device.
                            return handleDeviceLost();
                        }
                        else if (GAPIStatusU::Failure(result = GAPIStatus(hr)))
                        {
                            LOG_ERROR("Failed ResizeBuffers")
                            return GAPIStatus(result);
                        }
                    }
                    else
                    {
                        const auto& graphicsCommandQueue = getCommandQueue(CommandQueueType::GRAPHICS);
                        const auto& targetSwapChainDesc = D3DUtils::GetDXGISwapChainDesc1(presentOptions, DXGI_SWAP_EFFECT_FLIP_DISCARD);

                        ComSharedPtr<IDXGISwapChain1> swapChain2;
                        // Create a swap chain for the window.
                        if (GAPIStatusU::Failure(result = GAPIStatus(_dxgiFactory->CreateSwapChainForHwnd(
                                                     graphicsCommandQueue.get(),
                                                     presentOptions.windowHandle,
                                                     &targetSwapChainDesc,
                                                     nullptr,
                                                     nullptr,
                                                     swapChain2.put()))))
                        {
                            LOG_ERROR("Failure CreateSwapChainForHwnd");
                            return result;
                        }

                        swapChain2.as(_swapChain);
                    }

                    // Update backbuffers.
                    DXGI_SWAP_CHAIN_DESC1 currentSwapChainDesc;
                    if (GAPIStatusU::Failure(result = GAPIStatus(_swapChain->GetDesc1(&currentSwapChainDesc))))
                    {
                        LOG_ERROR("Failure get swapChain Desc");
                        return result;
                    }

                    for (UINT n = 0; n < _backBufferCount; n++)
                    {
                        ThrowIfFailed(_swapChain->GetBuffer(n, IID_PPV_ARGS(_renderTargets[n].put())));

                        //TODO
                        wchar_t name[25] = {};
                        swprintf_s(name, L"Render target %u", n);
                        _renderTargets[n]->SetName(name);

                        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                        rtvDesc.Format = currentSwapChainDesc.Format;
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

                        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(
                            _rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                            static_cast<INT>(n), _rtvDescriptorSize);
                        _d3dDevice->CreateRenderTargetView(_renderTargets[n].get(), &rtvDesc, rtvDescriptor);
                    }

                    _backBufferIndex = 0;

                    // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
                    //   if (GAPIStatusU::Failure(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER)))
                    // {
                    // }*/
                }

                GAPIStatus DeviceImplementation::Present()
                {
                    ASSERT_IS_CREATION_THREAD;

                    GAPIStatus result = GAPIStatus::OK;

                    const auto& commandQueue = getCommandQueue(CommandQueueType::GRAPHICS);

                    const UINT64 currentFenceValue = _fenceValues[_frameIndex];

                    {
                        // Transition the render target to the state that allows it to be presented to the display.
                        // D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_backBufferIndex].Get(), beforeState, D3D12_RESOURCE_STATE_PRESENT);
                        //commandList->ResourceBarrier(1, &barrier);
                    }

                    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[_backBufferIndex].get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
                    const auto& commandList = _commandList->GetCommandList();

                    commandList->ResourceBarrier(1, &barrier);
                    for (int i = 0; i < 100000; i++)
                    {
                        float color[4] = { std::rand() / static_cast<float>(RAND_MAX),
                            std::rand() / static_cast<float>(RAND_MAX),
                            std::rand() / static_cast<float>(RAND_MAX), 1 };

                        commandList->ClearRenderTargetView(getRenderTargetView(_backBufferIndex), color, 0, nullptr);
                    }
                    barrier = CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[_backBufferIndex].get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
                    commandList->ResourceBarrier(1, &barrier);
                    // Send the command list off to the GPU for processing.
                    commandList->Close();

                    //  TODO Check correct fence work.
                    _commandList->Submit(commandQueue.get());

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
                    HRESULT hr = _swapChain->Present1(0, 0, &parameters);

                    // If the device was reset we must completely reinitialize the renderer.
                    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
                    {
                        Log::Print::Warning("Device Lost on Present: Reason code 0x%08X\n", static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? _d3dDevice->GetDeviceRemovedReason() : hr));

                        // Todo error check
                        handleDeviceLost();
                    }
                    else
                    {
                        if (GAPIStatusU::Failure(result = GAPIStatus(hr)))
                        {
                            LOG_ERROR("Fail on Present");
                            return result;
                        }

                        moveToNextFrame();

                        if (!_dxgiFactory->IsCurrent())
                        {
                            LOG_ERROR("Dxgi is not current");
                            return GAPIStatus::FAIL;

                            // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
                            //ThrowIfFailed(CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(m_dxgiFactory.ReleaseAndGetAddressOf())));
                        }
                    }

                    return result;
                }

                GAPIStatus DeviceImplementation::handleDeviceLost()
                {
                    // Todo implement properly Device lost event processing
                    Log::Print::Fatal("Device was lost.");
                    return GAPIStatus::OK;
                }

                GAPIStatus DeviceImplementation::createDevice()
                {
                    ASSERT_IS_CREATION_THREAD;
                    GAPIStatus result;

                    UINT dxgiFactoryFlags = 0;
                    // Enable the debug layer (requires the Graphics Tools "optional feature").
                    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
                    if (_enableDebug)
                    {
                        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(_debugController.put()))))
                        {
                            _debugController->EnableDebugLayer();
                            _debugController->SetEnableGPUBasedValidation(true);
                            _debugController->SetEnableSynchronizedCommandQueueValidation(true);
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

                    if (GAPIStatusU::Failure(result = GAPIStatus(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(_dxgiFactory.put())))))
                    {
                        LOG_ERROR("Failure create DXGIFactory with HRESULT of 0x%08X", result);
                        return result;
                    }

                    D3D_FEATURE_LEVEL minimumFeatureLevel = D3D_FEATURE_LEVEL_11_0;
                    if (GAPIStatusU::Failure(result = GAPIStatus(D3DUtils::GetAdapter(_dxgiFactory, minimumFeatureLevel, _dxgiAdapter))))
                    {
                        LOG_ERROR("Failure create Adapter with HRESULT of 0x%08X", result);
                        return result;
                    }

                    // Create the DX12 API device object.
                    if (GAPIStatusU::Failure(result = GAPIStatus(D3D12CreateDevice(_dxgiAdapter.get(), minimumFeatureLevel, IID_PPV_ARGS(_d3dDevice.put())))))
                    {
                        LOG_ERROR("Failure create Device with HRESULT of 0x%08X", result);
                        return result;
                    }

                    _d3dDevice->SetName(L"DX12Device");
                    if (_enableDebug)
                    {
                        // Configure debug device (if active).
                        ComSharedPtr<ID3D12InfoQueue> d3dInfoQueue;

                        if (GAPIStatusU::Success(result = GAPIStatus(
                                                     _d3dDevice->QueryInterface(IID_PPV_ARGS(d3dInfoQueue.put())))))
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

                    if (SUCCEEDED(_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels))))
                    {
                        _d3dFeatureLevel = featLevels.MaxSupportedFeatureLevel;
                    }
                    else
                    {
                        _d3dFeatureLevel = minimumFeatureLevel;
                    }

                    return result;
                }

                void DeviceImplementation::moveToNextFrame()
                {
                    const auto& commandQueue = getCommandQueue(CommandQueueType::GRAPHICS);

                    // Schedule a Signal command in the queue.
                    const UINT64 currentFenceValue = _fenceValues[_frameIndex];
                    // ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

                    _fence->Signal(commandQueue.get(), currentFenceValue);

                    // Update the back buffer index.
                    _backBufferIndex = _swapChain->GetCurrentBackBufferIndex();
                    _frameIndex = (_frameIndex++ % GPU_FRAMES_BUFFERED);

                    // If the next frame is not ready to be rendered yet, wait until it is ready.
                    if (_fence->GetGpuValue() < _fenceValues[_frameIndex])
                    {
                        _fence->SetEventOnCompletion(_fenceValues[_frameIndex], _fenceEvent.get());
                        //   ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_backBufferIndex], m_fenceEvent.Get()));
                        WaitForSingleObjectEx(_fenceEvent.get(), INFINITE, FALSE);
                    }

                    // Set the fence value for the next frame.
                    _fenceValues[_frameIndex] = currentFenceValue + 1;
                }

                Device::Device()
                    : _impl(new DeviceImplementation())
                {
                }

                Device::~Device() { }

                GAPIStatus Device::Init()
                {
                    return _impl->Init();
                }

                GAPIStatus Device::Reset(const PresentOptions& presentOptions)
                {
                    return _impl->Reset(presentOptions);
                }

                GAPIStatus Device::Present()
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

                GAPIStatus Device::InitResource(CommandList& commandList) const
                {
                    return GAPIStatus::OK;
                }
            }
        }
    }
}