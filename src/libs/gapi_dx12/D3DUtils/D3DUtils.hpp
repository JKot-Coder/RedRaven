#pragma once

#include "gapi/ForwardDeclarations.hpp"

#include "gapi_dx12/D3DUtils/DXGIFormatsUtils.hpp"

using HRESULT = long;

namespace RR
{
    namespace GAPI
    {
        struct PresentOptions;

        namespace DX12
        {
            namespace D3DUtils
            {

#define D3DCall(exp)                                                         \
    {                                                                        \
        const HRESULT result = exp;                                          \
        if (FAILED(result))                                                  \
        {                                                                    \
            LOG_FATAL(#exp " Error: {}", D3DUtils::HResultToString(result)); \
        }                                                                    \
    }

                template <typename T>
                struct D3D12TypeName
                {
                    static std::string name;
                };

                template <> std::string D3D12TypeName<ID3D12Fence>::name = "Fence";
                template <> std::string D3D12TypeName<ID3D12Device>::name = "Device";
                template <> std::string D3D12TypeName<ID3D12GraphicsCommandList>::name = "GraphicsCommandList";
                template <> std::string D3D12TypeName<ID3D12GraphicsCommandList4>::name = "GraphicsCommandList4";
                template <> std::string D3D12TypeName<ID3D12CommandAllocator>::name = "Allocator";
                template <> std::string D3D12TypeName<ID3D12Resource>::name = "GPUResource";
                template <> std::string D3D12TypeName<ID3D12DescriptorHeap>::name = "DescriptorHeap";
                template <> std::string D3D12TypeName<ID3D12CommandQueue>::name = "CommandQueue";
                template <> std::string D3D12TypeName<IDXGISwapChain3>::name = "SwapChain3";

                template <typename T>
                inline std::string GetTypeName()
                {
                    using Type = typename std::remove_pointer<T>::type;

                    static_assert(std::is_base_of<ID3D12Object, Type>::value || std::is_base_of<IDXGIObject, Type>::value,
                                  "Not a ID3D12Object or IDXGIObject");

                    return D3D12TypeName<Type>::name;
                }
#ifdef ENABLE_API_OBJECT_NAMES
                template <typename T>
                inline void SetAPIName(const T& apiObject, const std::string& name, int32_t index = -1)
                {
                   // const std::string formatedName = fmt::format((index > 0) ? "{0}::{1}_{:02}" : "{}::{}", GetTypeName<T>(), name, index);
                    const std::string formatedName = fmt::format("{}", name);
                    apiObject->SetName(StringEncoding::UTF8ToWide(formatedName).c_str());
                }
#else
                template <typename T>
                inline void SetAPIName(const T&, const std::string&)
                {
                }
                template <typename T>
                inline void SetAPIName(const T&, const std::string&, int32_t)
                {
                }
#endif
                std::string HResultToString(HRESULT hr);

                D3D12_RESOURCE_FLAGS GetResourceFlags(GpuResourceBindFlags flags);
                D3D12_RESOURCE_DESC GetResourceDesc(const GpuResourceDescription& resourceDesc);
                uint32_t GetSubresourcesCount(const D3D12_RESOURCE_DESC& desc);

                bool SwapChainDesc1MatchesForReset(const DXGI_SWAP_CHAIN_DESC1& left, const DXGI_SWAP_CHAIN_DESC1& right);
                DXGI_SWAP_CHAIN_DESC1 GetDxgiSwapChainDesc1(const PresentOptions& presentOptions, DXGI_SWAP_EFFECT swapEffect);
                DXGI_SWAP_CHAIN_DESC1 GetDxgiSwapChainDesc1(const SwapChainDescription& description, DXGI_SWAP_EFFECT swapEffect);

                HRESULT GetAdapter(const ComSharedPtr<IDXGIFactory1>& dxgiFactory, D3D_FEATURE_LEVEL minimumFeatureLevel, ComSharedPtr<IDXGIAdapter1>& Adapter);
            }
        }
    }
}