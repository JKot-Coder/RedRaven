#pragma once

using HRESULT = long;

namespace OpenDemo
{
    namespace Render
    {
        namespace Device
        {
            struct PresentOptions;

            namespace DX12
            {
                namespace D3DUtils
                {

                    bool SwapChainDesc1MatchesForReset(const DXGI_SWAP_CHAIN_DESC1& left, const DXGI_SWAP_CHAIN_DESC1& right);

                    DXGI_SWAP_CHAIN_DESC1 GetDXGISwapChainDesc1(const PresentOptions& presentOptions, DXGI_SWAP_EFFECT swapEffect);

                    DXGI_FORMAT SRGBToLinear(DXGI_FORMAT format);

                    HRESULT GetAdapter(const ComSharedPtr<IDXGIFactory1>& dxgiFactory, D3D_FEATURE_LEVEL minimumFeatureLevel, ComSharedPtr<IDXGIAdapter1>& Adapter);

                }
            }
        }
    }
}