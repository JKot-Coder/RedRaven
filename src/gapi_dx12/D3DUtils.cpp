#include "D3DUtils.hpp"

#include "gapi/DeviceInterface.hpp"
#include "gapi/SwapChain.hpp"

#include "gapi_dx12/TypeConversions.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            namespace D3DUtils
            {
                bool SwapChainDesc1MatchesForReset(const DXGI_SWAP_CHAIN_DESC1& left, const DXGI_SWAP_CHAIN_DESC1& right)
                {
                    return (left.Stereo == right.Stereo
                        && left.SampleDesc.Count == right.SampleDesc.Count
                        && left.SampleDesc.Quality == right.SampleDesc.Quality
                        && left.BufferUsage == right.BufferUsage
                        && left.SwapEffect == right.SwapEffect
                        && left.Flags == right.Flags);
                }

                DXGI_SWAP_CHAIN_DESC1 GetDXGISwapChainDesc1(const SwapChainDescription& description, DXGI_SWAP_EFFECT swapEffect)
                {
                    ASSERT(description.width > 0);
                    ASSERT(description.height > 0);
                    ASSERT(description.bufferCount > 0 && description.bufferCount <= MAX_BACK_BUFFER_COUNT);

                    DXGI_SWAP_CHAIN_DESC1 output;
                    output.Width = description.width;
                    output.Height = description.height;
                    output.Format = TypeConversions::GetResourceFormat(description.resourceFormat);
                    output.Stereo = (description.isStereo) ? TRUE : FALSE;
                    output.SampleDesc = { 1, 0 };
                    output.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                    output.BufferCount = description.bufferCount;
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