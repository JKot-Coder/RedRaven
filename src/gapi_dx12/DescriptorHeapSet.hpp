#pragma once

#include "DescriptorHeap.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            class DescriptorHeapSet
            {
            public:
                DescriptorHeapSet() = default;

                Result Init(ID3D12Device* device);

                DescriptorHeap::SharedPtr GetRtvDescriptorHeap() 
                { 
                    return rtvDescriptorHeap_;
                }

            private:
                DescriptorHeap::SharedPtr rtvDescriptorHeap_;
            };
        }
    }
}