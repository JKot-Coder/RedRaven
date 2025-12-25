#include "DeviceImpl.hpp"

#include "gapi/Buffer.hpp"
#include "gapi/CommandList2.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/Device.hpp"
#include "gapi/Fence.hpp"
#include "gapi/Frame.hpp"
#include "gapi/GpuResource.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

#include "gapi_dx12/CommandList2Impl.hpp"
#include "gapi_dx12/CommandQueueImpl.hpp"
#include "gapi_dx12/DescriptorHeap.hpp"
#include "gapi_dx12/DescriptorManager.hpp"
#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/ResourceCreator.hpp"
#include "gapi_dx12/ResourceImpl.hpp"
#include "gapi_dx12/ResourceReleaseContext.hpp"
#include "gapi_dx12/SwapChainImpl.hpp"
#include "gapi_dx12/third_party/d3d12_memory_allocator/D3D12MemAlloc.h"

#include <atomic>
#include <chrono>
#include <iterator>
#include <thread>

#ifdef ENABLE_ASSERTS
#define ASSERT_IS_CREATION_THREAD ASSERT(creationThreadID_ == std::this_thread::get_id())
#define ASSERT_IS_DEVICE_INITED ASSERT(inited_)
#else
#define ASSERT_IS_CREATION_THREAD
#define ASSERT_IS_DEVICE_INITED
#endif

#define NOT_IMPLEMENTED() ASSERT_MSG(false, "Not implemented")

using namespace std::chrono_literals;

namespace RR
{
    namespace GAPI
    {
        namespace DX12
        {
            template <typename T>
            void ThrowIfFailed(T c) { std::ignore = c; };

            DeviceImpl::DeviceImpl()
                : creationThreadID_(std::this_thread::get_id())
            {
            }

            DeviceImpl::~DeviceImpl()
            {
                ASSERT_IS_CREATION_THREAD;

                if (!inited_)
                    return;

                // Todo need wait all queries
                waitForGpu();
                waitForGpu();

               // InitialDataUploder::Instance().Terminate();
                DescriptorManager::Instance().Terminate();
                ResourceReleaseContext::Instance().Terminate();

                DeviceContext::GetGraphicsCommandQueue()->ImmediateD3DObjectRelease();
                DeviceContext::Terminate();
                gpuWaitFence_ = nullptr;

                dxgiFactory_ = nullptr;
                dxgiAdapter_ = nullptr;

                if (description_.debugMode != DeviceDesc::DebugMode::Retail)
                {
                    ComSharedPtr<ID3D12DebugDevice> debugLayer;

                    d3dDevice_->QueryInterface(IID_PPV_ARGS(debugLayer.put()));
                    d3dDevice_ = nullptr;

                    if (debugLayer)
                    {
                        Log::Print::Info("Dx12 leaked objects report:\n");
                        debugLayer->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
                    }
                }

                d3dDevice_ = nullptr;
            }

            bool DeviceImpl::Init(const DeviceDesc& description)
            {
                ASSERT_IS_CREATION_THREAD;
                ASSERT(!inited_);

                description_ = description;

                if (!createDevice())
                    return false;

                DeviceContext::Init(d3dDevice_, dxgiFactory_);

                gpuWaitFence_ = std::make_unique<FenceImpl>();
                gpuWaitFence_->Init("GpuWait");

                D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
                allocatorDesc.pDevice = d3dDevice_.get();
                allocatorDesc.pAdapter = dxgiAdapter_.get();

                D3D12MA::Allocator* allocator;
                D3D12MA::CreateAllocator(&allocatorDesc, &allocator);

                auto graphicsCommandQueue = std::make_shared<CommandQueueImpl>(CommandQueueType::Graphics);
                graphicsCommandQueue->Init("Primary");

                ResourceReleaseContext::Instance().Init();
                DescriptorManager::Instance().Init();
                //InitialDataUploder::Instance().Init();

                DeviceContext::Init(
                    allocator,
                    graphicsCommandQueue);

                inited_ = true;

                return true;
            }

            void DeviceImpl::waitForGpu()
            {
                ASSERT_IS_DEVICE_INITED;
                ASSERT(gpuWaitFence_);

                DeviceContext::GetGraphicsCommandQueue()->Signal(*gpuWaitFence_);
                gpuWaitFence_->Wait(std::nullopt);
                ResourceReleaseContext::ExecuteDeferredDeletions(DeviceContext::GetGraphicsCommandQueue());
            }

            GpuResourceFootprint DeviceImpl::GetResourceFootprint(const GpuResourceDesc& description) const
            {
                ASSERT_IS_DEVICE_INITED;
                return ResourceImpl::GetFootprint(description);
            }

            void DeviceImpl::InitSwapChain(SwapChain& resource) const
            {
                ASSERT_IS_DEVICE_INITED;
                return ResourceCreator::InitSwapChain(resource);
            }

            void DeviceImpl::InitFence(Fence& resource) const
            {
                ASSERT_IS_DEVICE_INITED;
                return ResourceCreator::InitFence(resource);
            }

            void DeviceImpl::InitCommandQueue(CommandQueue& resource) const
            {
                ASSERT_IS_DEVICE_INITED;
                return ResourceCreator::InitCommandQueue(resource);
            }

            void DeviceImpl::InitCommandList2(CommandList2& resource) const
            {
                ASSERT_IS_DEVICE_INITED;
                return ResourceCreator::InitCommandList2(resource);
            }

            void DeviceImpl::InitTexture(Texture& resource) const
            {
                ASSERT_IS_DEVICE_INITED;

                auto impl = std::make_unique<ResourceImpl>();
                impl->Init(resource);

                resource.SetPrivateImpl(impl.release());
            }

            void DeviceImpl::InitBuffer(Buffer& resource, const BufferData* initialData) const
            {
                ASSERT_IS_DEVICE_INITED;

                auto impl = std::make_unique<ResourceImpl>();
                impl->Init(resource);

                resource.SetPrivateImpl(impl.release());
            }

            void DeviceImpl::InitShader(Shader& resource) const
            {
                ASSERT_IS_DEVICE_INITED;
                NOT_IMPLEMENTED();
            }

            void DeviceImpl::InitPipelineState(PipelineState& resource) const
            {
                ASSERT_IS_DEVICE_INITED;
                NOT_IMPLEMENTED();
            }

            void DeviceImpl::InitGpuResourceView(GpuResourceView& view) const
            {
                ASSERT_IS_DEVICE_INITED;
                return ResourceCreator::InitGpuResourceView(view);
            }

            void DeviceImpl::Compile(CommandList2& commandList)
            {
                ASSERT_IS_DEVICE_INITED;

                auto commandListImpl = static_cast<CommandList2Impl*>(commandList.GetPrivateImpl());
                commandListImpl->Compile(commandList);
            }
            /*
            void DeviceImpl::Submit(const CommandList::SharedPtr& commandList)
            {
                ASSERT_IS_CREATION_THREAD;
                ASSERT_IS_DEVICE_INITED;
                ASSERT(commandList);

                Log::Print::Info("submit\n");
                const auto& commandQueue = getCommandQueue(CommandQueueType::Graphics);

                ASSERT(dynamic_cast<CommandListImpl*>(commandList->GetPrivateImpl()));
                const auto commandListImpl = reinterpret_cast<const CommandListImpl*>(commandList->GetPrivateImpl());

                const auto D3DCommandList = commandListImpl->GetD3DObject();
                ASSERT(D3DCommandList);

                ID3D12CommandList* ppCommandLists[] = { D3DCommandList.get() };
                commandQueue->ExecuteCommandLists(std::size(ppCommandLists), ppCommandLists);

            }*/

            void DeviceImpl::Present(SwapChain* swapChain)
            {
                ASSERT_IS_CREATION_THREAD;
                ASSERT_IS_DEVICE_INITED;
                ASSERT(swapChain);

                /* HRESULT hr;
                 if (m_options & c_AllowTearing)
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
                // DXGI_PRESENT_PARAMETERS parameters = {};

                ASSERT(dynamic_cast<SwapChainImpl*>(swapChain->GetPrivateImpl()));
                auto swapChainImpl = static_cast<SwapChainImpl*>(swapChain->GetPrivateImpl());

                auto result = swapChainImpl->Present(0);

                // If the device was reset we must completely reinitialize the renderer.
                if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET)
                {
                    if(result == DXGI_ERROR_DEVICE_REMOVED)
                    {
                        HRESULT undelayedError = d3dDevice_->GetDeviceRemovedReason();
                        LOG_FATAL("Device Lost on Present. Error: {}", D3DUtils::HResultToString(undelayedError));
                    }
                    // Todo error check
                    // handleDeviceLost();
                    ASSERT(false);
                }
                else
                {
                    if (!dxgiFactory_->IsCurrent())
                    {
                        LOG_FATAL("Dxgi is not current");

                        // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
                        // ThrowIfFailed(CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(m_dxgiFactory.ReleaseAndGetAddressOf())));
                    }
                }
            }

            bool DeviceImpl::createDevice()
            {
                ASSERT_IS_CREATION_THREAD;

                UINT dxgiFactoryFlags = 0;

                // Enable the debug layer (requires the Graphics Tools "optional feature").
                // NOTE: Enabling the debug layer after device creation will invalidate the active device.
                if (description_.debugMode == DeviceDesc::DebugMode::Debug || description_.debugMode == DeviceDesc::DebugMode::Instrumented)
                {
                    ComSharedPtr<ID3D12Debug1> debugController;
                    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.put()))))
                    {
                        debugController->EnableDebugLayer();
                        debugController->SetEnableGPUBasedValidation(true);

                        if (description_.debugMode == DeviceDesc::DebugMode::Debug)
                        {
                            debugController->SetEnableSynchronizedCommandQueueValidation(true);
                        }
                    }
                    else
                    {
                        LOG_WARNING("Direct3D Debug Device is not available");
                    }

                    ComSharedPtr<IDXGIInfoQueue> dxgiInfoQueue;
                    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.put()))))
                    {
                        dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

                        dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
                        dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
                    }
                }

                HRESULT result;
                if (FAILED(result = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(dxgiFactory_.put()))))
                {
                    LOG_WARNING("Failed to create DXGIFactory. Error: {}", D3DUtils::HResultToString(result));
                    return false;
                }

                D3D_FEATURE_LEVEL minimumFeatureLevel = D3D_FEATURE_LEVEL_11_0;
                if (FAILED(result = D3DUtils::GetAdapter(dxgiFactory_, minimumFeatureLevel, dxgiAdapter_)))
                {
                    LOG_WARNING("Failed to get adapter. Error: {}", D3DUtils::HResultToString(result));
                    return false;
                }

                // Create the DX12 API device object.
                if (FAILED(result = D3D12CreateDevice(dxgiAdapter_.get(), minimumFeatureLevel, IID_PPV_ARGS(d3dDevice_.put()))))
                {
                    LOG_WARNING("Failed to CreateDevice. Error: {}", D3DUtils::HResultToString(result));
                    return false;
                }

                D3DUtils::SetAPIName(d3dDevice_.get(), "Main");

                if (description_.debugMode == DeviceDesc::DebugMode::Debug || description_.debugMode == DeviceDesc::DebugMode::Instrumented)
                {
                    // Configure debug device (if active).
                    ComSharedPtr<ID3D12InfoQueue> d3dInfoQueue;
                    bool clearDefaultFilters = false;

                    if (SUCCEEDED(result = d3dDevice_->QueryInterface(IID_PPV_ARGS(d3dInfoQueue.put()))))
                    {
                        if (clearDefaultFilters)
                        {
                            d3dInfoQueue->ClearRetrievalFilter();
                            d3dInfoQueue->ClearStorageFilter();
                        }

                        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
                        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
                    }
                    else
                    {
                        LOG_WARNING("Unable to get ID3D12InfoQueue. Error: {}", D3DUtils::HResultToString(result));
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
                    _countof(s_featureLevels), s_featureLevels, D3D_FEATURE_LEVEL_11_0};

                if (SUCCEEDED(d3dDevice_->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels))))
                {
                    d3dFeatureLevel_ = featLevels.MaxSupportedFeatureLevel;
                }
                else
                {
                    d3dFeatureLevel_ = minimumFeatureLevel;
                }

                return true;
            }

            void DeviceImpl::MoveToNextFrame(uint64_t frameIndex)
            {
                ASSERT_IS_CREATION_THREAD;
                ASSERT_IS_DEVICE_INITED;

                ResourceReleaseContext::ExecuteDeferredDeletions(DeviceContext::GetGraphicsCommandQueue());
                DeviceContext::GetAllocator()->SetCurrentFrameIndex(frameIndex);
            }

            /*
            void DeviceImpl::DefferedDeleteD3DObject(const ComSharedPtr<IUnknown>& object)
            {
                ASSERT(defferedDeletionResources_);

                // Device already destoyed. Leaked object destruction.
                if (!defferedDeletionResources_)
                    return;

                defferedDeletionResources_->push_back(object);
            }*/
        }
    }
}