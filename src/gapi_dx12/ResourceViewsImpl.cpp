#include "ResourceViewsImpl.hpp"

#include "gapi/ResourceViews.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        { /*
            Result RenderTargetViewImpl::Init(ID3D12Device* device, const RenderTargetView& rtv, const DescriptorHandle& handle)
            {
               
                if (DSVDesc.Format == TEX_FORMAT_UNKNOWN)
                {
                    DSVDesc.Format = m_Desc.Format;
                }

                D3D12_DEPTH_STENCIL_VIEW_DESC D3D12_DSVDesc;
                TextureViewDesc_to_D3D12_DSV_DESC(DSVDesc, D3D12_DSVDesc, m_Desc.SampleCount);

                auto* pDeviceD3D12 = static_cast<RenderDeviceD3D12Impl*>(GetDevice())->GetD3D12Device();
                pDeviceD3D12->CreateDepthStencilView(m_pd3d12Resource, &D3D12_DSVDesc, DSVHandle);

                return Result::OK;
            }*/
        }
    }
}