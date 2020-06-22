#include "Device.hpp"

#include "gapi_dx12/D3DUtils.hpp"
#include "gapi_dx12/d3dx12.h"

#include <vector>

#include <chrono>
#include <thread>

#ifdef ENABLE_ASSERTS
#define ASSERT_IS_CREATION_THREAD ASSERT(_privateData->creationThreadID == std::this_thread::get_id())
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

                struct PrivateDeviceData
                {
                    ComSharedPtr<ID3D12Debug1> debugController;
                    ComSharedPtr<IDXGIFactory2> dxgiFactory;
                    ComSharedPtr<IDXGIAdapter1> dxgiAdapter;
                    ComSharedPtr<ID3D12Device> d3dDevice;
                    std::thread::id creationThreadID;

                    D3D_FEATURE_LEVEL d3dFeatureLevel = D3D_FEATURE_LEVEL_1_0_CORE;
                };

                class TemporaryDX12Impl
                {
                public:
                    static const int MAX_GPU_FRAMES_BUFFERED = 3;
                    static const int MAX_BACK_BUFFER_COUNT = 3;

                    TemporaryDX12Impl(PrivateDeviceData* privateData)
                        : _privateData(privateData)
                    {
                    }

                    GAPIStatus HandleDeviceLost();
                    GAPIStatus Reset(const PresentOptions& presentOptions);
                    GAPIStatus Init();
                    GAPIStatus Present();
                    CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView();

                    void MoveToNextFrame();

                    void WaitForGpu();

                private:
                    PrivateDeviceData* _privateData;
                    ComSharedPtr<ID3D12CommandQueue> _commandQueue;
                    ComSharedPtr<ID3D12DescriptorHeap> _rtvDescriptorHeap;
                    std::array<ComSharedPtr<ID3D12CommandAllocator>, MAX_GPU_FRAMES_BUFFERED> m_commandAllocators;
                    ComSharedPtr<ID3D12GraphicsCommandList> m_commandList;
                    ComSharedPtr<ID3D12Fence> m_fence;
                    uint32_t m_framesBuffered = 2;
                    UINT64 m_fenceValues[MAX_GPU_FRAMES_BUFFERED];
                    winrt::handle m_fenceEvent;
                    uint32_t m_rtvDescriptorSize;
                    uint32_t m_backBufferIndex = 0;
                    uint32_t m_backBufferCount = 0;
                    uint32_t m_frameIndex = 0;
                    ComSharedPtr<IDXGISwapChain3> _swapChain;
                    ComSharedPtr<ID3D12Resource> m_renderTargets[MAX_BACK_BUFFER_COUNT];
                };

                void TemporaryDX12Impl::WaitForGpu()
                {
                    // TODO. Do nothing by now
                }

                void TemporaryDX12Impl::MoveToNextFrame()
                {
                    // Schedule a Signal command in the queue.
                    const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
                    // ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));
                    _commandQueue->Signal(m_fence.get(), currentFenceValue);

                    // Update the back buffer index.
                    m_backBufferIndex = _swapChain->GetCurrentBackBufferIndex();
                    m_frameIndex = (m_frameIndex++ % m_framesBuffered);

                    // If the next frame is not ready to be rendered yet, wait until it is ready.
                    if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
                    {
                        m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent.get());
                        //   ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_backBufferIndex], m_fenceEvent.Get()));
                        WaitForSingleObjectEx(m_fenceEvent.get(), INFINITE, FALSE);
                    }

                    // Set the fence value for the next frame.
                    m_fenceValues[m_frameIndex] = currentFenceValue + 1;
                }

                CD3DX12_CPU_DESCRIPTOR_HANDLE TemporaryDX12Impl::GetRenderTargetView()
                {
                    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
                        _rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                        static_cast<INT>(m_backBufferIndex), m_rtvDescriptorSize);
                }

                GAPIStatus TemporaryDX12Impl::Present()
                {
                    GAPIStatus result = GAPIStatus::OK;
                    const auto& device = _privateData->d3dDevice;
                    const auto& commandList = m_commandList;
                    const auto& commandQueue = _commandQueue;
                    const auto& swapChain = _swapChain;
                    const auto& dxgiFactory = _privateData->dxgiFactory;

                    static int index = 0;
                    // if (beforeState != D3D12_RESOURCE_STATE_PRESENT)
                    m_commandAllocators[index]->Reset();
                    m_commandList->Reset(m_commandAllocators[index].get(), nullptr);

                    index = (index++ % m_framesBuffered);

                    {
                        // Transition the render target to the state that allows it to be presented to the display.
                        // D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_backBufferIndex].Get(), beforeState, D3D12_RESOURCE_STATE_PRESENT);
                        //commandList->ResourceBarrier(1, &barrier);
                    }

                    float color[4] = { std::rand() / static_cast<float>(RAND_MAX),
                        std::rand() / static_cast<float>(RAND_MAX),
                        std::rand() / static_cast<float>(RAND_MAX), 1 };

                    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_backBufferIndex].get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
                    commandList->ResourceBarrier(1, &barrier);
                    commandList->ClearRenderTargetView(GetRenderTargetView(), color, 0, nullptr);
                    barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_backBufferIndex].get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
                    commandList->ResourceBarrier(1, &barrier);
                    // Send the command list off to the GPU for processing.   */
                    m_commandList->Close();
                    ID3D12CommandList* cc[] = { static_cast<ID3D12CommandList*>(m_commandList.get()) };
                    commandQueue->ExecuteCommandLists(1, cc);

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
                    HRESULT hr = swapChain->Present1(0, 0, &parameters);

                    // If the device was reset we must completely reinitialize the renderer.
                    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
                    {
                        Log::Print::Warning("Device Lost on Present: Reason code 0x%08X\n", static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? device->GetDeviceRemovedReason() : hr));

                        // Todo error check
                        HandleDeviceLost();
                    }
                    else
                    {
                        if (GAPIStatusU::Failure(result = GAPIStatus(hr)))
                        {
                            LOG_ERROR("Fail on Present");
                            return result;
                        }

                        MoveToNextFrame();

                        if (!dxgiFactory->IsCurrent())
                        {
                            LOG_ERROR("Dxgi is not current");
                            return GAPIStatus::FAIL;

                            // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
                            //ThrowIfFailed(CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(m_dxgiFactory.ReleaseAndGetAddressOf())));
                        }
                    }

                    return result;
                }

                GAPIStatus TemporaryDX12Impl::Init()
                {
                    ASSERT_IS_CREATION_THREAD;

                    GAPIStatus result = GAPIStatus::OK;

                    // Create the command queue.
                    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
                    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
                    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

                    if (GAPIStatusU::Failure(result = GAPIStatus(_privateData->d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(_commandQueue.put())))))
                    {
                        LOG_ERROR("Failure create CommandQueue with HRESULT of 0x%08X", result);
                        return result;
                    }
                    _commandQueue->SetName(L"MainCommanQueue");

                    // Create descriptor heaps for render target views and depth stencil views.
                    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
                    rtvDescriptorHeapDesc.NumDescriptors = MAX_BACK_BUFFER_COUNT;
                    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

                    if (GAPIStatusU::Failure(result = GAPIStatus(_privateData->d3dDevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(_rtvDescriptorHeap.put())))))
                    {
                        LOG_ERROR("Failure create DescriptorHeap with HRESULT of 0x%08X", result);
                        return result;
                    }

                    _rtvDescriptorHeap->SetName(L"DescriptorHead");

                    m_rtvDescriptorSize = _privateData->d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
                    /*
                    if (m_depthBufferFormat != DXGI_FORMAT_UNKNOWN)
                    {
                        D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
                        dsvDescriptorHeapDesc.NumDescriptors = 1;
                        dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

                        ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(m_dsvDescriptorHeap.ReleaseAndGetAddressOf())));

                        m_dsvDescriptorHeap->SetName(L"DeviceResources");
                    }*/

                    // Create a command allocator for each back buffer that will be rendered to.
                    for (UINT n = 0; n < m_framesBuffered; n++)
                    {
                        if (GAPIStatusU::Failure(result = GAPIStatus(_privateData->d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocators[n].put())))))
                        {
                            LOG_ERROR("Failure create CreateCommandAllocator with HRESULT of 0x%08X", result);
                            return result;
                        }

                        m_commandAllocators[n]->SetName(StringConversions::UTF8ToWString(fmt::sprintf("Render target %u", n)).c_str());
                    }

                    // Create a command list for recording graphics commands.
                    if (GAPIStatusU::Failure(result = GAPIStatus(_privateData->d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].get(), nullptr, IID_PPV_ARGS(m_commandList.put())))))
                    {
                        return result;
                    }

                    if (GAPIStatusU::Failure(result = GAPIStatus(m_commandList->Close())))
                    {
                        return result;
                    }

                    m_commandList->SetName(L"CommandList");

                    // Create a fence for tracking GPU execution progress.
                    if (GAPIStatusU::Failure(result = GAPIStatus(_privateData->d3dDevice->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.put())))))
                    {
                        LOG_ERROR("Failure create CreateFence with HRESULT of 0x%08X", result);
                        return result;
                    }

                    m_fenceValues[m_frameIndex]++;
                    m_fence->SetName(L"DeviceResources");

                    m_fenceEvent.attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));
                    if (!bool { m_fenceEvent })
                    {
                        LOG_ERROR("Failure create fence Event");
                        return GAPIStatus::FAIL;
                    }
                }

                GAPIStatus TemporaryDX12Impl::HandleDeviceLost()
                {
                    // Todo implement properly Device lost event processing
                    Log::Print::Fatal("Device was lost.");
                    return GAPIStatus::OK;
                }

                template <typename T>
                void ThrowIfFailed(T c) { std::ignore = c; };

                // These resources need to be recreated every time the window size is changed.
                GAPIStatus TemporaryDX12Impl::Reset(const PresentOptions& presentOptions)
                {
                    ASSERT_IS_CREATION_THREAD;
                    ASSERT(presentOptions.windowHandle);

                    if (!m_backBufferCount)
                        m_backBufferCount = presentOptions.bufferCount;

                    ASSERT_MSG(presentOptions.bufferCount == m_backBufferCount, "Changing backbuffer count should work, but this is untested")

                    const HWND windowHandle = presentOptions.windowHandle;

                    if (!windowHandle)
                    {
                        LOG_ERROR("Call SetWindow with a valid Win32 window handle");
                        return GAPIStatus::FAIL;
                    }

                    // Wait until all previous GPU work is complete.
                    WaitForGpu();

                    GAPIStatus result = GAPIStatus::OK;

                    // Release resources that are tied to the swap chain and update fence values.
                    for (int n = 0; n < m_framesBuffered; n++)
                    {
                        m_renderTargets[n] = nullptr;
                        m_fenceValues[n] = m_fenceValues[m_frameIndex];
                    }

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
                                static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? _privateData->d3dDevice->GetDeviceRemovedReason() : hr))

                            // If the device was removed for any reason, a new device and swap chain will need to be created.
                            // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method
                            // and correctly set up the new device.
                            return HandleDeviceLost();
                        }
                        else if (GAPIStatusU::Failure(result = GAPIStatus(hr)))
                        {
                            LOG_ERROR("Failed ResizeBuffers")
                            return GAPIStatus(result);
                        }
                    }
                    else
                    {
                        const auto& targetSwapChainDesc = D3DUtils::GetDXGISwapChainDesc1(presentOptions, DXGI_SWAP_EFFECT_FLIP_DISCARD);

                        ComSharedPtr<IDXGISwapChain1> swapChain;
                        // Create a swap chain for the window.
                        if (GAPIStatusU::Failure(result = GAPIStatus(_privateData->dxgiFactory->CreateSwapChainForHwnd(
                                                     _commandQueue.get(),
                                                     presentOptions.windowHandle,
                                                     &targetSwapChainDesc,
                                                     nullptr,
                                                     nullptr,
                                                     swapChain.put()))))
                        {
                            LOG_ERROR("Failure CreateSwapChainForHwnd");
                            return result;
                        }

                        swapChain.as(_swapChain);

                        // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
                        //   if (GAPIStatusU::Failure(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER)))
                        // {
                        // }*/
                    }

                    DXGI_SWAP_CHAIN_DESC1 currentSwapChainDesc;
                    if (GAPIStatusU::Failure(result = GAPIStatus(_swapChain->GetDesc1(&currentSwapChainDesc))))
                    {
                        LOG_ERROR("Failure get swapChain Desc");
                        return result;
                    }

                    for (UINT n = 0; n < m_backBufferCount; n++)
                    {
                        ThrowIfFailed(_swapChain->GetBuffer(n, IID_PPV_ARGS(m_renderTargets[n].put())));

                        wchar_t name[25] = {};
                        swprintf_s(name, L"Render target %u", n);
                        m_renderTargets[n]->SetName(name);

                        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                        rtvDesc.Format = currentSwapChainDesc.Format;
                        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

                        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(
                            _rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                            static_cast<INT>(n), m_rtvDescriptorSize);
                        _privateData->d3dDevice->CreateRenderTargetView(m_renderTargets[n].get(), &rtvDesc, rtvDescriptor);
                    }

                    m_backBufferIndex = 0;
                }

                GAPIStatus Device::Present()
                {
                    ASSERT_IS_CREATION_THREAD;

                    return _impl->Present();
                }

                Device::Device()
                    : _privateData(new PrivateDeviceData())
                    , _impl(new TemporaryDX12Impl(_privateData.get()))
                {
                    _privateData->creationThreadID = std::this_thread::get_id();
                }

                Device::~Device() { }

                GAPIStatus Device::CreateDevice()
                {
                    ASSERT_IS_CREATION_THREAD;
                    GAPIStatus result;

                    UINT dxgiFactoryFlags = 0;
                    // Enable the debug layer (requires the Graphics Tools "optional feature").
                    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
                    if (_enableDebug)
                    {
                        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(_privateData->debugController.put()))))
                        {
                            _privateData->debugController->EnableDebugLayer();
                            _privateData->debugController->SetEnableGPUBasedValidation(true);
                            _privateData->debugController->SetEnableSynchronizedCommandQueueValidation(true);
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

                    if (GAPIStatusU::Failure(result = GAPIStatus(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(_privateData->dxgiFactory.put())))))
                    {
                        LOG_ERROR("Failure create DXGIFactory with HRESULT of 0x%08X", result);
                        return result;
                    }

                    D3D_FEATURE_LEVEL minimumFeatureLevel = D3D_FEATURE_LEVEL_11_0;
                    if (GAPIStatusU::Failure(result = GAPIStatus(D3DUtils::GetAdapter(_privateData->dxgiFactory, minimumFeatureLevel, _privateData->dxgiAdapter))))
                    {
                        LOG_ERROR("Failure create Adapter with HRESULT of 0x%08X", result);
                        return result;
                    }

                    // Create the DX12 API device object.
                    if (GAPIStatusU::Failure(result = GAPIStatus(D3D12CreateDevice(_privateData->dxgiAdapter.get(), minimumFeatureLevel, IID_PPV_ARGS(_privateData->d3dDevice.put())))))
                    {
                        LOG_ERROR("Failure create Device with HRESULT of 0x%08X", result);
                        return result;
                    }

                    _privateData->d3dDevice->SetName(L"DX12Device");
                    if (_enableDebug)
                    {
                        // Configure debug device (if active).
                        ComSharedPtr<ID3D12InfoQueue> d3dInfoQueue;

                        if (GAPIStatusU::Success(result = GAPIStatus(
                                                     _privateData->d3dDevice->QueryInterface(IID_PPV_ARGS(d3dInfoQueue.put())))))
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

                    if (SUCCEEDED(_privateData->d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels))))
                    {
                        _privateData->d3dFeatureLevel = featLevels.MaxSupportedFeatureLevel;
                    }
                    else
                    {
                        _privateData->d3dFeatureLevel = minimumFeatureLevel;
                    }

                    return result;
                }

                GAPIStatus Device::Init()
                {
                    ASSERT_IS_CREATION_THREAD;

                    GAPIStatus result = GAPIStatus::OK;

                    if (GAPIStatusU::Failure(result = CreateDevice()))
                    {
                        LOG_ERROR("Failed CreateDevice");
                        return result;
                    }

                    if (GAPIStatusU::Failure(result = _impl->Init()))
                    {
                        LOG_ERROR("Failed in private impl Init");
                        return result;
                    };
                }

                GAPIStatus Device::Reset(const PresentOptions& presentOptions)
                {
                    ASSERT_IS_CREATION_THREAD;

                    return _impl->Reset(presentOptions);
                }
            }
        }
    }
}