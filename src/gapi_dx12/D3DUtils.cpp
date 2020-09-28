#include "D3DUtils.hpp"

#include "common/Logger.hpp"

#include "gapi/DeviceInterface.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            namespace D3DUtils
            {

                DXGI_FORMAT getDXGIFormat(ResourceFormat resourceFormat)
                {
                    // Check it's not implemented yet;
                    ASSERT(resourceFormat == ResourceFormat::Unknown)

                    return DXGI_FORMAT_B8G8R8A8_UNORM;
                }

                bool SwapChainDesc1MatchesForReset(const DXGI_SWAP_CHAIN_DESC1& left, const DXGI_SWAP_CHAIN_DESC1& right)
                {
                    return (left.Stereo == right.Stereo
                        && left.SampleDesc.Count == right.SampleDesc.Count
                        && left.SampleDesc.Quality == right.SampleDesc.Quality
                        && left.BufferUsage == right.BufferUsage
                        && left.SwapEffect == right.SwapEffect
                        && left.Flags == right.Flags);
                }

                DXGI_SWAP_CHAIN_DESC1 GetDXGISwapChainDesc1(const PresentOptions& presentOptions, DXGI_SWAP_EFFECT swapEffect)
                {
                    ASSERT(presentOptions.rect.width > 0);
                    ASSERT(presentOptions.rect.height > 0);
                    ASSERT(presentOptions.rect.left == 0);
                    ASSERT(presentOptions.rect.top == 0);
                    ASSERT(presentOptions.bufferCount > 0 && presentOptions.bufferCount <= MAX_BACK_BUFFER_COUNT);

                    DXGI_SWAP_CHAIN_DESC1 output;
                    output.Width = presentOptions.rect.width;
                    output.Height = presentOptions.rect.height;
                    output.Format = getDXGIFormat(presentOptions.resourceFormat);
                    output.Stereo = (presentOptions.isStereo) ? TRUE : FALSE;
                    output.SampleDesc = { 1, 0 };
                    output.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                    output.BufferCount = presentOptions.bufferCount;
                    output.Scaling = DXGI_SCALING_STRETCH;
                    output.SwapEffect = swapEffect;
                    output.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
                    output.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
                    return output;
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

                // Todo replace to Result
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