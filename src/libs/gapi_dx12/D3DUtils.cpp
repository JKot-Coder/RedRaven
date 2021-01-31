#include "D3DUtils.hpp"

#include "gapi/Buffer.hpp"
#include "gapi/Device.hpp"
#include "gapi/SwapChain.hpp"
#include "gapi/Texture.hpp"

#include "gapi_dx12/TypeConversions.hpp"

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

                U8String HResultToString(HRESULT hr)
                {
                    if (SUCCEEDED(hr))
                        return "";

                    const auto pMessage = ErrorMessage(hr);
                    const auto& messageString = StringConversions::WStringToUTF8(pMessage);
                    LocalFree((HLOCAL)pMessage);

                    return messageString;
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

                DXGI_SWAP_CHAIN_DESC1 GetDXGISwapChainDesc1(const SwapChainDescription& description, DXGI_SWAP_EFFECT swapEffect)
                {
                    ASSERT(description.width > 0);
                    ASSERT(description.height > 0);
                    ASSERT(description.bufferCount > 0 && description.bufferCount <= MAX_BACK_BUFFER_COUNT);

                    DXGI_SWAP_CHAIN_DESC1 output;
                    output.Width = description.width;
                    output.Height = description.height;
                    output.Format = TypeConversions::GetGpuResourceFormat(description.gpuResourceFormat);
                    output.Stereo = (description.isStereo) ? TRUE : FALSE;
                    output.SampleDesc = { 1, 0 };
                    output.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                    output.BufferCount = description.bufferCount;
                    output.Scaling = DXGI_SCALING_STRETCH;
                    output.SwapEffect = swapEffect;
                    output.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
                    output.Flags = 0;
                    return output;
                }

                D3D12_RESOURCE_DESC GetResourceDesc(const TextureDescription& resourceDesc, GpuResourceBindFlags bindFlags)
                {
                    DXGI_FORMAT format = TypeConversions::GetGpuResourceFormat(resourceDesc.format);

                    if (GpuResourceFormatInfo::IsDepth(resourceDesc.format) && IsAny(bindFlags, GpuResourceBindFlags::ShaderResource | GpuResourceBindFlags::UnorderedAccess))
                        format = TypeConversions::GetTypelessFormatFromDepthFormat(resourceDesc.format);

                    D3D12_RESOURCE_DESC desc;
                    switch (resourceDesc.dimension)
                    {
                    case TextureDimension::Texture1D:
                        desc = CD3DX12_RESOURCE_DESC::Tex1D(format, resourceDesc.width, resourceDesc.arraySize, resourceDesc.mipLevels);
                        break;
                    case TextureDimension::Texture2D:
                    case TextureDimension::Texture2DMS:
                        desc = CD3DX12_RESOURCE_DESC::Tex2D(format, resourceDesc.width, resourceDesc.height, resourceDesc.arraySize, resourceDesc.mipLevels, resourceDesc.sampleCount);
                        break;
                    case TextureDimension::Texture3D:
                        desc = CD3DX12_RESOURCE_DESC::Tex3D(format, resourceDesc.width, resourceDesc.height, resourceDesc.depth, resourceDesc.mipLevels);
                        break;
                    case TextureDimension::TextureCube:
                        desc = CD3DX12_RESOURCE_DESC::Tex2D(format, resourceDesc.width, resourceDesc.height, resourceDesc.arraySize * 6, resourceDesc.mipLevels);
                        break;
                    default:
                        LOG_FATAL("Unsupported texture dimension");
                    }

                    desc.Flags = TypeConversions::GetResourceFlags(bindFlags);
                    return desc;
                }

                D3D12_RESOURCE_DESC GetResourceDesc(const BufferDescription& resourceDesc, GpuResourceBindFlags bindFlags)
                {
                    D3D12_RESOURCE_DESC desc;

                    desc = CD3DX12_RESOURCE_DESC::Buffer(resourceDesc.size);
                    desc.Flags = TypeConversions::GetResourceFlags(bindFlags);
                    desc.Format = TypeConversions::GetGpuResourceFormat(resourceDesc.format);
                    return desc;
                }

                DXGI_FORMAT SRGBToLinear(DXGI_FORMAT format)
                {
                    switch (format)
                    {
                    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
                        return DXGI_FORMAT_R8G8B8A8_UNORM;
                    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
                        return DXGI_FORMAT_B8G8R8A8_UNORM;
                    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
                        return DXGI_FORMAT_B8G8R8X8_UNORM;
                    default:
                        return format;
                    }
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
                            Log::Print::Info("Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, StringConversions::WStringToUTF8(desc.Description));
                            break;
                        }
                    }

                    return result;
                }
            }
        }
    }
}