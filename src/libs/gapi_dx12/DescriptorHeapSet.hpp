#pragma once

#include "DescriptorHeap.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            class DescriptorHeapSet
            {
            public:
                DescriptorHeapSet() = default;

                Result Init();

                const DescriptorHeap::SharedPtr& GetRtvDescriptorHeap() const { return rtvDescriptorHeap_; }

            private:
                DescriptorHeap::SharedPtr rtvDescriptorHeap_;
            };
        }
    }
}