#pragma once

#include "gapi/ForwardDeclarations.hpp"

#include "gapi_dx12/D3DUtils/DXGIFormatsUtils.hpp"

using HRESULT = long;

namespace OpenDemo
{
    namespace GAPI
    {
        struct PresentOptions;

        namespace DX12
        {
            namespace D3DUtils
            {

#define D3DCall(exp, ...)                                                                                         \
    {                                                                                                             \
        static_assert(std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value == 0, "Too many arguments"); \
        const HRESULT result = exp;                                                                               \
        if (FAILED(result))                                                                                       \
        {                                                                                                         \
            LOG_FATAL(#exp " Error: %s", D3DUtils::HResultToString(result))                                       \
        }                                                                                                         \
    }

#ifdef ENABLE_API_OBJECT_NAMES
                template <typename T>
                struct D3D12TypeName
                {
                    static U8String name;
                };

                template <>
                U8String D3D12TypeName<ID3D12Fence>::name = "Fence";
                U8String D3D12TypeName<ID3D12Device>::name = "Device";
                U8String D3D12TypeName<ID3D12GraphicsCommandList>::name = "CommandList";
                U8String D3D12TypeName<ID3D12CommandAllocator>::name = "Allocator";
                U8String D3D12TypeName<ID3D12Resource>::name = "Resource";
                U8String D3D12TypeName<ID3D12DescriptorHeap>::name = "DescriptorHeap";
                U8String D3D12TypeName<ID3D12CommandQueue>::name = "CommandQueue";

                template <typename T>
                inline void SetAPIName(const T& apiObject, const U8String& name)
                {
                    using Type = std::remove_pointer<T>::type;
                    static_assert(std::is_base_of<ID3D12Object, Type>::value, "Wrong type for FormatAPIName");
                    apiObject->SetName(StringConversions::UTF8ToWString(fmt::format(FMT_STRING("{}::{}"), D3D12TypeName<Type>::name, name)).c_str());
                }

                template <typename T>
                inline void SetAPIName(const T& apiObject, const U8String& name, int index)
                {
                    using Type = std::remove_pointer<T>::type;
                    static_assert(std::is_base_of<ID3D12Object, Type>::value, "Wrong type for FormatAPIName");
                    apiObject->SetName(StringConversions::UTF8ToWString(fmt::format(FMT_STRING("{}::{}_{:02}"), D3D12TypeName<Type>::name, name, index)).c_str());
                }
#else
                template <typename T>
                inline void SetAPIName(const T&, const U8String&)
                {
                }
                template <typename T>
                inline void SetAPIName(const T&, const U8String&, int)
                {
                }
#endif
                U8String HResultToString(HRESULT hr);

                D3D12_RESOURCE_FLAGS GetResourceFlags(GpuResourceBindFlags flags);
                D3D12_RESOURCE_DESC GetResourceDesc(const GpuResourceDescription& resourceDesc);

                bool SwapChainDesc1MatchesForReset(const DXGI_SWAP_CHAIN_DESC1& left, const DXGI_SWAP_CHAIN_DESC1& right);
                DXGI_SWAP_CHAIN_DESC1 GetDxgiSwapChainDesc1(const PresentOptions& presentOptions, DXGI_SWAP_EFFECT swapEffect);
                DXGI_SWAP_CHAIN_DESC1 GetDxgiSwapChainDesc1(const SwapChainDescription& description, DXGI_SWAP_EFFECT swapEffect);

                HRESULT GetAdapter(const ComSharedPtr<IDXGIFactory1>& dxgiFactory, D3D_FEATURE_LEVEL minimumFeatureLevel, ComSharedPtr<IDXGIAdapter1>& Adapter);
            }
        }
    }
}