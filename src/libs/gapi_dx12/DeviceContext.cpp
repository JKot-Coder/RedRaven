#include "DeviceContext.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            ComSharedPtr<ID3D12Device> DeviceContext::device_;
            ComSharedPtr<IDXGIFactory2> DeviceContext::dxgiFactory_;
            std::shared_ptr<CommandQueueImpl> DeviceContext::graphicsCommandQueue_;
            std::shared_ptr<DescriptorHeapSet> DeviceContext::descriptorHeapSet_;
            std::shared_ptr<ResourceReleaseContext> DeviceContext::resourceReleaseContext_;
            std::shared_ptr<GpuMemoryHeap> DeviceContext::uploadHeap_;
            std::shared_ptr<GpuMemoryHeap> DeviceContext::readbackHeap_;
        }
    }
}