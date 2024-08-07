#include "D3DUtils.hpp"

#include "gapi/Buffer.hpp"
#include "gapi/Device.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

#include <comdef.h>

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            namespace D3DUtils
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
                        size_t const nLen = strlen(m_pszMsg);
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
                    const auto& messageString = Common::StringEncoding::WideToUTF8(pMessage);
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

                D3D12_RESOURCE_DESC GetResourceDesc(const GpuResourceDescription& resourceDesc)
                {
                    DXGI_FORMAT format = GetDxgiResourceFormat(resourceDesc.GetFormat());

                    if (GpuResourceFormatInfo::IsDepth(resourceDesc.GetFormat()) && IsAny(resourceDesc.GetBindFlags(), GpuResourceBindFlags::ShaderResource | GpuResourceBindFlags::UnorderedAccess))
                        format = GetDxgiTypelessFormat(resourceDesc.GetFormat());

                    D3D12_RESOURCE_DESC desc;
                    switch (resourceDesc.GetDimension())
                    {
                        case GpuResourceDimension::Texture1D:
                            desc = CD3DX12_RESOURCE_DESC::Tex1D(format, resourceDesc.GetWidth(), resourceDesc.GetArraySize(), resourceDesc.GetMipCount());
                            break;
                        case GpuResourceDimension::Texture2D:
                        case GpuResourceDimension::Texture2DMS:
                            desc = CD3DX12_RESOURCE_DESC::Tex2D(format, resourceDesc.GetWidth(), resourceDesc.GetHeight(), resourceDesc.GetArraySize(), resourceDesc.GetMipCount(), resourceDesc.GetSampleCount());
                            break;
                        case GpuResourceDimension::Texture3D:
                            desc = CD3DX12_RESOURCE_DESC::Tex3D(format, resourceDesc.GetWidth(), resourceDesc.GetHeight(), resourceDesc.GetDepth(), resourceDesc.GetMipCount());
                            break;
                        case GpuResourceDimension::TextureCube:
                            desc = CD3DX12_RESOURCE_DESC::Tex2D(format, resourceDesc.GetWidth(), resourceDesc.GetHeight(), resourceDesc.GetArraySize() * 6, resourceDesc.GetMipCount());
                            break;
                        default:
                            LOG_FATAL("Unsupported texture dimension");
                    }

                    desc.Flags = GetResourceFlags(resourceDesc.GetBindFlags());
                    return desc;
                }

                /*
                * TODO BUFFER SUPPORT
                D3D12_RESOURCE_DESC GetResourceDesc(const BufferDescription& resourceDesc)
                {
                    D3D12_RESOURCE_DESC desc;

                    ASSERT_MSG(false, "Fix bindflags")
                    desc = CD3DX12_RESOURCE_DESC::Buffer(resourceDesc.size);
                    desc.Flags = TypeConversions::GetResourceFlags(GpuResourceBindFlags::None);
                    desc.Format = TypeConversions::GetGpuResourceFormat(resourceDesc.format);
                    return desc;
                }*/

                bool SwapChainDesc1MatchesForReset(const DXGI_SWAP_CHAIN_DESC1& left, const DXGI_SWAP_CHAIN_DESC1& right)
                {
                    return (left.Stereo == right.Stereo &&
                            left.SampleDesc.Count == right.SampleDesc.Count &&
                            left.SampleDesc.Quality == right.SampleDesc.Quality &&
                            left.BufferUsage == right.BufferUsage &&
                            left.SwapEffect == right.SwapEffect &&
                            left.Flags == right.Flags);
                }

                DXGI_SWAP_CHAIN_DESC1 GetDxgiSwapChainDesc1(const SwapChainDescription& description, DXGI_SWAP_EFFECT swapEffect)
                {
                    ASSERT(description.width >= 0);
                    ASSERT(description.height >= 0);
                    ASSERT(description.bufferCount > 0 && description.bufferCount <= MAX_BACK_BUFFER_COUNT);

                    DXGI_SWAP_CHAIN_DESC1 output;
                    output.Width = description.width;
                    output.Height = description.height;
                    output.Format = GetDxgiResourceFormat(description.gpuResourceFormat);
                    output.Stereo = (description.isStereo) ? TRUE : FALSE;
                    output.SampleDesc = {1, 0};
                    output.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                    output.BufferCount = description.bufferCount;
                    output.Scaling = DXGI_SCALING_STRETCH;
                    output.SwapEffect = swapEffect;
                    output.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
                    output.Flags = 0;
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
                        {
                            // Don't select the software adapter.
                            continue;
                        }

                        // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
                        if (SUCCEEDED(result = D3D12CreateDevice(adapter.get(), minimumFeatureLevel, _uuidof(ID3D12Device), nullptr)))
                        {
                            Log::Print::Info("Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, Common::StringEncoding::WideToUTF8(desc.Description));
                            break;
                        }
                    }

                    return result;
                }
            }
        }
    }
}