#include "DeviceContext.hpp"

#include "gapi_dx12/third_party/d3d12_memory_allocator/D3D12MemAlloc.h"

namespace RR
{
    namespace GAPI
    {
        namespace DX12
        {
            D3D12MA::Allocator* DeviceContext::allocator_;
            ComSharedPtr<ID3D12Device> DeviceContext::device_;
            ComSharedPtr<IDXGIFactory2> DeviceContext::dxgiFactory_;
            std::shared_ptr<CommandQueueImpl> DeviceContext::graphicsCommandQueue_;

            void DeviceContext::Init(const ComSharedPtr<ID3D12Device>& device,
                                     const ComSharedPtr<IDXGIFactory2>& dxgiFactory)
            {
                ASSERT(device);
                ASSERT(dxgiFactory);

                device_ = device;
                dxgiFactory_ = dxgiFactory;
            }

            void DeviceContext::Init(D3D12MA::Allocator* allocator,
                                     const std::shared_ptr<CommandQueueImpl>& graphicsCommandQueue,
                                     std::shared_ptr<InitialDataUploder>& initialDataUploder)
            {
                ASSERT(allocator);
                ASSERT(graphicsCommandQueue);
                ASSERT(initialDataUploder);

                allocator_ = allocator;
                graphicsCommandQueue_ = graphicsCommandQueue;
                initialDataUploder_ = initialDataUploder;
            }

            void DeviceContext::Terminate()
            {
                allocator_->Release();
                allocator_ = nullptr;
                device_ = nullptr;
                dxgiFactory_ = nullptr;
                graphicsCommandQueue_ = nullptr;
            }
        }
    }
}