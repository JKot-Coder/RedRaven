#pragma once

#include "common/Singleton.hpp"

#include "gapi/GpuResource.hpp"

#include "DescriptorHeap.hpp"

namespace RR
{
    namespace GAPI
    {
        namespace DX12
        {
            class DescriptorManager final : public Singleton<DescriptorManager>
            {
            public:
                DescriptorManager() = default;

                void Init();
                void Terminate();

                void Allocate(GpuResourceView& resourceView);

                const DescriptorHeap::Descriptor& GetNullRtvDescriptor(GpuResourceDimension dimension) { return nullRtvDescriptors_[size_t(dimension)]; }
                //      const GetNullRtvDescriptor() const { nullRtvDescriptor_.GetCPUHandle(); }

                const DescriptorHeap::SharedPtr& GetRtvDescriptorHeap() const { return rtvDescriptorHeap_; }

            private:
                bool isInited_ = false;
                std::array<DescriptorHeap::Descriptor, size_t(GpuResourceDimension::Count)> nullRtvDescriptors_;
                DescriptorHeap::SharedPtr rtvDescriptorHeap_;
                DescriptorHeap::SharedPtr cbvUavSrvDescriptorHeap_;
            };
        }
    }
}