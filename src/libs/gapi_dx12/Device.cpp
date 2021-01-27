#include "Device.hpp"

#include "gapi/CommandList.hpp"
#include "gapi/CommandQueue.hpp"
#include "gapi/Device.hpp"
#include "gapi/Fence.hpp"
#include "gapi/Frame.hpp"
#include "gapi/Object.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

#include "gapi_dx12/CommandListImpl.hpp"
#include "gapi_dx12/CommandQueueImpl.hpp"
#include "gapi_dx12/D3DUtils.hpp"
#include "gapi_dx12/DescriptorHeap.hpp"
#include "gapi_dx12/DescriptorHeapSet.hpp"
#include "gapi_dx12/DeviceContext.hpp"
#include "gapi_dx12/FenceImpl.hpp"
#include "gapi_dx12/GpuMemoryHeap.hpp"
#include "gapi_dx12/ResourceCreator.hpp"
#include "gapi_dx12/ResourceImpl.hpp"
#include "gapi_dx12/ResourceReleaseContext.hpp"
#include "gapi_dx12/SwapChainImpl.hpp"

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

using namespace std::chrono_literals;

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            template <typename T>
            void ThrowIfFailed(T c) { std::ignore = c; };

            ComSharedPtr<ID3D12Device> DeviceContext::device_;
            ComSharedPtr<IDXGIFactory2> DeviceContext::dxgiFactory_;
            std::shared_ptr<CommandQueueImpl> DeviceContext::graphicsCommandQueue_;
            std::shared_ptr<DescriptorHeapSet> DeviceContext::descriptorHeapSet_;
            std::shared_ptr<ResourceReleaseContext> DeviceContext::resourceReleaseContext_;
            std::shared_ptr<GpuMemoryHeap> DeviceContext::uploadHeap_;

            class DeviceImpl final : public IDevice
            {
            public:
                DeviceImpl();
                virtual ~DeviceImpl();

                void Init(const IDevice::Description& description) override;
                void Submit(const CommandList::SharedPtr& commandList);
                void Present(const SwapChain::SharedPtr& swapChain) override;
                void MoveToNextFrame() override;
                void WaitForGpu() override;

                void InitSwapChain(SwapChain& resource) const override;
                void InitFence(Fence& resource) const override;
                void InitCommandQueue(CommandQueue& resource) const override;
                void InitCommandList(CommandList& resource) const override;
                void InitTexture(Texture& resource, const std::vector<TextureSubresourceFootprint>& subresourcesFootprint) const override;
                void InitBuffer(Buffer& resource) const override;
                void InitGpuResourceView(GpuResourceView& view) const override;

                void ReleaseResource(Object& resource) const override;

                ID3D12Device* GetDevice() const
                {
                    return d3dDevice_.get();
                }

            private:
                void createDevice();

            private:
                IDevice::Description description_ = {};

                std::atomic_bool inited_ = false;

                std::thread::id creationThreadID_;

                D3D_FEATURE_LEVEL d3dFeatureLevel_ = D3D_FEATURE_LEVEL_1_0_CORE;

                ComSharedPtr<IDXGIFactory2> dxgiFactory_;
                ComSharedPtr<IDXGIAdapter1> dxgiAdapter_;
                ComSharedPtr<ID3D12Device> d3dDevice_;
                std::shared_ptr<FenceImpl> gpuWaitFence_;
            };

            DeviceImpl::DeviceImpl()
                : creationThreadID_(std::this_thread::get_id())
            {
            }

            DeviceImpl::~DeviceImpl()
            {
                ASSERT_IS_CREATION_THREAD;

                if (!inited_)
                    return;

                WaitForGpu();

                DeviceContext::Terminate();

                gpuWaitFence_ = nullptr;
                dxgiFactory_ = nullptr;
                dxgiAdapter_ = nullptr;

                if (description_.debugMode != IDevice::DebugMode::Retail)
                {
                    ComSharedPtr<ID3D12DebugDevice> debugLayer;

                    d3dDevice_->QueryInterface(IID_PPV_ARGS(debugLayer.put()));
                    d3dDevice_ = nullptr;

                    ASSERT(debugLayer);

                    if (debugLayer)
                    {
                        Log::Print::Info("Dx12 leaked objects report:\n");
                        debugLayer->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
                    }
                }

                d3dDevice_ = nullptr;
            }

            void DeviceImpl::Init(const IDevice::Description& description)
            {
                // TODO TODO TODO TODO returnvalue
                ASSERT_IS_CREATION_THREAD;
                ASSERT(!inited_);

                ASSERT(description.gpuFramesBuffered <= MAX_BACK_BUFFER_COUNT);

                description_ = description;

                createDevice();

                DeviceContext::Init(d3dDevice_, dxgiFactory_);

                auto& descriptorHeapSet = std::make_shared<DescriptorHeapSet>();
                descriptorHeapSet->Init();

                gpuWaitFence_ = std::make_unique<FenceImpl>();
                gpuWaitFence_->Init("GpuWait");

                auto& graphicsCommandQueue = std::make_shared<CommandQueueImpl>(CommandQueueType::Graphics);
                graphicsCommandQueue->Init("Primary");

                auto& resourceReleaseContext = std::make_shared<ResourceReleaseContext>();
                resourceReleaseContext->Init();

                constexpr size_t UploadHeapPageSize = 1024 * 1024 * 64; //64 Mb
                auto& uploadHeap = std::make_shared<GpuMemoryHeap>(UploadHeapPageSize);
                uploadHeap->Init("Upload heap");

                DeviceContext::Init(
                    graphicsCommandQueue,
                    descriptorHeapSet,
                    resourceReleaseContext,
                    uploadHeap);

                inited_ = true;
            }

            void DeviceImpl::WaitForGpu()
            {
                ASSERT_IS_DEVICE_INITED;

                gpuWaitFence_->Signal(*DeviceContext::GetGraphicsCommandQueue().get());
                gpuWaitFence_->SyncCPU(std::nullopt);
                DeviceContext::GetResourceReleaseContext()->ExecuteDeferredDeletions(DeviceContext::GetGraphicsCommandQueue());
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

            void DeviceImpl::InitCommandList(CommandList& resource) const
            {
                ASSERT_IS_DEVICE_INITED;
                return ResourceCreator::InitCommandList(resource);
            }

            void DeviceImpl::InitTexture(Texture& resource, const std::vector<TextureSubresourceFootprint>& subresourcesFootprint) const
            {
                ASSERT_IS_DEVICE_INITED;

                auto impl = std::make_unique<ResourceImpl>();
                impl->Init(resource, subresourcesFootprint);

                resource.SetPrivateImpl(impl.release());
            }

            void DeviceImpl::InitBuffer(Buffer& resource) const
            {
                ASSERT_IS_DEVICE_INITED;

                auto impl = std::make_unique<ResourceImpl>();
                impl->Init(resource);

                resource.SetPrivateImpl(impl.release());
            }

            void DeviceImpl::InitGpuResourceView(GpuResourceView& view) const
            {
                ASSERT_IS_DEVICE_INITED;
                return ResourceCreator::InitGpuResourceView(view);
            }

            void DeviceImpl::ReleaseResource(Object& resource) const
            {
                ASSERT_IS_DEVICE_INITED;
                return ResourceCreator::ReleaseResource(resource);
            }

            void DeviceImpl::Submit(const CommandList::SharedPtr& commandList)
            {
                /* ASSERT_IS_CREATION_THREAD;
                ASSERT_IS_DEVICE_INITED;
                ASSERT(commandList)

                Log::Print::Info("submit\n");
                const auto& commandQueue = getCommandQueue(CommandQueueType::Graphics);

                ASSERT(dynamic_cast<CommandListImpl*>(commandList->GetPrivateImpl()));
                const auto commandListImpl = reinterpret_cast<const CommandListImpl*>(commandList->GetPrivateImpl());

                const auto D3DCommandList = commandListImpl->GetD3DObject();
                ASSERT(D3DCommandList)

                ID3D12CommandList* ppCommandLists[] = { D3DCommandList.get() };
                commandQueue->ExecuteCommandLists(std::size(ppCommandLists), ppCommandLists);
                */
            }

            void DeviceImpl::Present(const SwapChain::SharedPtr& swapChain)
            {
                ASSERT_IS_CREATION_THREAD;
                ASSERT_IS_DEVICE_INITED;
                ASSERT(swapChain);

                HRESULT hr;
                /*  if (m_options & c_AllowTearing)
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
                DXGI_PRESENT_PARAMETERS parameters = {};

                ASSERT(dynamic_cast<SwapChainImpl*>(swapChain->GetPrivateImpl()));
                auto swapChainImpl = static_cast<SwapChainImpl*>(swapChain->GetPrivateImpl());
                    
                    
                    /// TODO TODO TODO TODO
              /*  void result = swapChainImpl->Present(0);

                // If the device was reset we must completely reinitialize the renderer.
                if (result == void ::DeviceRemoved || result == void ::DeviceReset)
                {
                    result = (result == void ::DeviceRemoved) ? void(d3dDevice_->GetDeviceRemovedReason()) : result;
                    Log::Print::Warning("Device Lost on Present. Error: %s\n", result.ToString());

                    // Todo error check
                    //handleDeviceLost();
                    ASSERT(false);

                    return void ::Fail;
                }
                else
                {
                    if (!dxgiFactory_->IsCurrent())
                    {
                        LOG_ERROR("Dxgi is not current");
                        return void ::Fail;

                        // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
                        //ThrowIfFailed(CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(m_dxgiFactory.ReleaseAndGetAddressOf())));
                    }
                }*/
            }

            void DeviceImpl::createDevice()
            {
                ASSERT_IS_CREATION_THREAD

                UINT dxgiFactoryFlags = 0;

                // Enable the debug layer (requires the Graphics Tools "optional feature").
                // NOTE: Enabling the debug layer after device creation will invalidate the active device.
                if (description_.debugMode == IDevice::DebugMode::Debug || description_.debugMode == IDevice::DebugMode::Instrumented)
                {
                    ComSharedPtr<ID3D12Debug1> debugController;
                    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.put()))))
                    {
                        debugController->EnableDebugLayer();
                        debugController->SetEnableGPUBasedValidation(true);

                        if (description_.debugMode == IDevice::DebugMode::Debug)
                        {
                            debugController->SetEnableSynchronizedCommandQueueValidation(true);
                        }
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

                if (description_.debugMode == IDevice::DebugMode::Debug || description_.debugMode == IDevice::DebugMode::Instrumented)
                {
                    // Configure debug device (if active).
                    ComSharedPtr<ID3D12InfoQueue> d3dInfoQueue;

                    // TODO TODO TODO TODO
                    /*
                    void result;
                    if (result = void(d3dDevice_->QueryInterface(IID_PPV_ARGS(d3dInfoQueue.put()))))
                    {
                        d3dInfoQueue->ClearRetrievalFilter();
                        d3dInfoQueue->ClearStorageFilter();

                        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
                        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
                    }
                    else
                    {
                        LOG_ERROR("Unable to get ID3D12InfoQueue. Error: %s", result.ToString());
                    }*/
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
            }

            void DeviceImpl::MoveToNextFrame()
            {
                ASSERT_IS_CREATION_THREAD;
                ASSERT_IS_DEVICE_INITED;

                DeviceContext::GetResourceReleaseContext()->ExecuteDeferredDeletions(DeviceContext::GetGraphicsCommandQueue());
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

            std::shared_ptr<Device> CreateDevice()
            {
                auto& device = Device::Create("Primary");
                ASSERT(device);

                device->SetPrivateImpl(new DeviceImpl());
                return device;
            }
        }
    }
}