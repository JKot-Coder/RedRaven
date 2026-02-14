#include "BindingGroupImpl.hpp"

#include "gapi/BindingGroupLayout.hpp"
#include "gapi/GpuResource.hpp"
#include "gapi/GpuResourceViews.hpp"

#include "BufferImpl.hpp"
#include "TextureImpl.hpp"
#include "TextureViewImpl.hpp"

namespace RR::GAPI::WebGPU
{
    BindingGroupImpl::~BindingGroupImpl() { }

    void BindingGroupImpl::Init(const wgpu::Device& device, BindingGroupDesc& desc)
    {
        eastl::fixed_vector<wgpu::BindGroupEntry, GAPI::MAX_BINDINGS_PER_GROUP, false> entries;
        for (const auto& element : desc.elements)
        {
            wgpu::BindGroupEntry entry;
            entry.setDefault();
            entry.binding = element.bindingIndex;

            switch (element.view->GetViewType())
            {
                case GAPI::GpuResourceView::ViewType::ShaderResourceView:
                {
                    const auto gpuResource = element.view->GetGpuResource();
                    ASSERT(gpuResource);

                    switch (gpuResource->GetDesc().GetDimension())
                    {
                        case GAPI::GpuResourceDimension::Buffer:
                        {
                            entry.buffer = gpuResource->GetPrivateImpl<BufferImpl>()->GetBuffer();
                            break;
                        }
                        case GAPI::GpuResourceDimension::Texture1D:
                        case GAPI::GpuResourceDimension::Texture2D:
                        case GAPI::GpuResourceDimension::Texture2DMS:
                        case GAPI::GpuResourceDimension::Texture3D:
                        case GAPI::GpuResourceDimension::TextureCube:
                        {
                            entry.textureView = element.view->GetPrivateImpl<TextureViewImpl>()->GetTextureView();
                            break;
                        }
                        default:
                        {
                            ASSERT_MSG(false, "Unsupported resource dimension");
                            break;
                        }
                    }
                    break;
                }
                default:
                {
                    ASSERT_MSG(false, "Unsupported resource view type");
                    break;
                }
            }
            entries.push_back(entry);
        }

        wgpu::BindGroupDescriptor bindGroupDesc;
        bindGroupDesc.setDefault();
        //bindGroupDesc.layout = ???;
        bindGroupDesc.entryCount = desc.elements.size();
        bindGroupDesc.entries = entries.data();

        device.createBindGroup(bindGroupDesc);

    }
}

