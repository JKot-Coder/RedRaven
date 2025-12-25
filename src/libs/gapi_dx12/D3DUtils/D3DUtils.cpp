#include "D3DUtils.hpp"

#include "gapi/Buffer.hpp"
#include "gapi/Device.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

#include <comdef.h>

namespace RR::GAPI::DX12::D3DUtils
{
    // Copy from _com_error::ErrorMessage(), with english locale
    inline const TCHAR* ErrorMessage(HRESULT hr) throw()
    {
        TCHAR* pszMsg;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_FROM_SYSTEM |
                          FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      hr,
                      MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
                      (LPTSTR)&pszMsg,
                      0,
                      NULL);
        if (pszMsg != NULL)
        {
#ifdef UNICODE
            size_t const nLen = wcslen(pszMsg);
#else
            size_t const nLen = strlen(pszMsg);
#endif
            if (nLen > 1 && pszMsg[nLen - 1] == '\n')
            {
                pszMsg[nLen - 1] = 0;
                if (pszMsg[nLen - 2] == '\r')
                {
                    pszMsg[nLen - 2] = 0;
                }
            }
        }
        else
        {
            pszMsg = (LPTSTR)LocalAlloc(0, 32 * sizeof(TCHAR));
            if (pszMsg != NULL)
            {
                WORD wCode = _com_error::HRESULTToWCode(hr);
                if (wCode != 0)
                {
                    _COM_PRINTF_S_1(pszMsg, 32, TEXT("IDispatch error #%d"), (int)wCode);
                }
                else
                {
                    _COM_PRINTF_S_1(pszMsg, 32, TEXT("Unknown error 0x%0lX"), hr);
                }
            }
        }
        return pszMsg;
    }

    std::string HResultToString(HRESULT hr)
    {
        if (SUCCEEDED(hr))
            return "";

        const auto pMessage = ErrorMessage(hr);
        const auto& messageString = Common::StringEncoding::WideToUTF8(std::wstring(pMessage));
        LocalFree((HLOCAL)pMessage);

        return messageString;
    }

    D3D12_RESOURCE_FLAGS GetResourceFlags(GpuResourceBindFlags flags)
    {
        D3D12_RESOURCE_FLAGS d3d = D3D12_RESOURCE_FLAG_NONE;

        d3d |= IsSet(flags, GpuResourceBindFlags::UnorderedAccess) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
        // Flags cannot have D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE set without D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
        d3d |= !IsSet(flags, GpuResourceBindFlags::ShaderResource) && IsSet(flags, GpuResourceBindFlags::DepthStencil) ? D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE : D3D12_RESOURCE_FLAG_NONE;
        d3d |= IsSet(flags, GpuResourceBindFlags::DepthStencil) ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_NONE;
        d3d |= IsSet(flags, GpuResourceBindFlags::RenderTarget) ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET : D3D12_RESOURCE_FLAG_NONE;

        return d3d;
    }

    D3D12_RESOURCE_DESC GetResourceDesc(const GpuResourceDesc& resourceDesc)
    {
        ASSERT(resourceDesc.IsValid());

        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

        if (resourceDesc.dimension != GpuResourceDimension::Buffer)
        {
            format = GetDxgiResourceFormat(resourceDesc.texture.format);

            if (GpuResourceFormatInfo::IsDepth(resourceDesc.texture.format) && IsAny(resourceDesc.bindFlags, GpuResourceBindFlags::ShaderResource | GpuResourceBindFlags::UnorderedAccess))
                format = GetDxgiTypelessFormat(resourceDesc.texture.format);

            ASSERT(format != DXGI_FORMAT_UNKNOWN);
        }

        D3D12_RESOURCE_DESC desc;
        switch (resourceDesc.dimension)
        {
            case GpuResourceDimension::Buffer: desc = CD3DX12_RESOURCE_DESC::Buffer(resourceDesc.buffer.size); break;
            case GpuResourceDimension::Texture1D: desc = CD3DX12_RESOURCE_DESC::Tex1D(format, resourceDesc.texture.width, resourceDesc.texture.arraySize, resourceDesc.texture.mipLevels); break;
            case GpuResourceDimension::Texture2D:
            case GpuResourceDimension::Texture2DMS:
            {
                const auto& sampleDesc = GetSampleDesc(resourceDesc.texture.multisampleType);
                desc = CD3DX12_RESOURCE_DESC::Tex2D(format, resourceDesc.texture.width, resourceDesc.texture.height, resourceDesc.texture.arraySize, resourceDesc.texture.mipLevels, sampleDesc.Count, sampleDesc.Quality);
                break;
            }
            case GpuResourceDimension::Texture3D: desc = CD3DX12_RESOURCE_DESC::Tex3D(format, resourceDesc.texture.width, resourceDesc.texture.height, resourceDesc.texture.depth, resourceDesc.texture.mipLevels); break;
            case GpuResourceDimension::TextureCube: desc = CD3DX12_RESOURCE_DESC::Tex2D(format, resourceDesc.texture.width, resourceDesc.texture.height, resourceDesc.texture.arraySize * 6, resourceDesc.texture.mipLevels); break;
            default: LOG_FATAL("Unsupported texture dimension");
        }

        desc.Flags = GetResourceFlags(resourceDesc.bindFlags);
        return desc;
    }

    uint32_t GetSubresourcesCount(const D3D12_RESOURCE_DESC& desc)
    {
        constexpr uint32_t planeSlices = 1;
        const auto arraySize = (desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D) ? desc.DepthOrArraySize : 1;
        return planeSlices * arraySize * desc.MipLevels;
    }

    bool SwapChainDesc1MatchesForReset(const DXGI_SWAP_CHAIN_DESC1& left, const DXGI_SWAP_CHAIN_DESC1& right)
    {
        return (left.Stereo == right.Stereo &&
                left.SampleDesc.Count == right.SampleDesc.Count &&
                left.SampleDesc.Quality == right.SampleDesc.Quality &&
                left.BufferUsage == right.BufferUsage &&
                left.SwapEffect == right.SwapEffect &&
                left.Flags == right.Flags);
    }

    DXGI_SWAP_CHAIN_DESC1 GetDxgiSwapChainDesc1(const SwapChainDesc& description, DXGI_SWAP_EFFECT swapEffect)
    {
        ASSERT(description.width >= 0);
        ASSERT(description.height >= 0);
        ASSERT(description.backBuffersCount > 0 && description.backBuffersCount <= MAX_BACK_BUFFERS_COUNT);

        DXGI_SWAP_CHAIN_DESC1 output;
        output.Width = description.width;
        output.Height = description.height;
        output.Format = GetDxgiResourceFormat(description.backBufferFormat);
        output.Stereo = FALSE;
        output.SampleDesc = {1, 0};
        output.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        output.BufferCount = description.backBuffersCount;
        output.Scaling = DXGI_SCALING_NONE;
        output.SwapEffect = swapEffect;
        output.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        output.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        return output;
    }

    // Todo replace to void
    HRESULT GetAdapter(const ComSharedPtr<IDXGIFactory1>& dxgiFactory, D3D_FEATURE_LEVEL minimumFeatureLevel, ComSharedPtr<IDXGIAdapter1>& adapter)
    {
        HRESULT result = E_FAIL;

        for (uint32_t adapterIndex = 0;; ++adapterIndex)
        {
            if (FAILED(result = dxgiFactory->EnumAdapters1(adapterIndex, adapter.put())))
                break;

            DXGI_ADAPTER_DESC1 desc;
            if (FAILED(result = adapter->GetDesc1(&desc)))
                break;

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                continue; // Don't select the software adapter.

            // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
            if (SUCCEEDED(result = D3D12CreateDevice(adapter.get(), minimumFeatureLevel, _uuidof(ID3D12Device), nullptr)))
            {
                Log::Print::Info("Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, Common::StringEncoding::WideToUTF8(std::wstring_view(desc.Description)));
                break;
            }
        }

        return result;
    }
}