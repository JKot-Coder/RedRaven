#pragma once

#include "common/Singleton.hpp"

#include "DescriptorHeap.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class DescriptorAllocator final : public Singleton<DescriptorAllocator>
            {
            public:
                DescriptorAllocator() = default;

                void Init();
                void Terminate();

                void Allocate(GpuResourceView& resourceView);

                const DescriptorHeap::SharedPtr& GetRtvDescriptorHeap() const { return rtvDescriptorHeap_; }

            private:
                bool isInited_ = false;
                DescriptorHeap::SharedPtr rtvDescriptorHeap_;
                DescriptorHeap::SharedPtr cbvUavSrvDescriptorHeap_;
            };
        }
    }
}