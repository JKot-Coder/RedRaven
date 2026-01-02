#include "BindingLayoutImpl.hpp"

#include "gapi/BindingLayout.hpp"
#include "gapi/GpuResource.hpp"

#include "Utils.hpp"

namespace RR::GAPI::WebGPU
{
    namespace
    {
        constexpr wgpu::ShaderStage getShaderStage(GAPI::ShaderStageMask stageMask)
        {
            wgpu::ShaderStage stages = wgpu::ShaderStage::None;
            if (Common::IsSet(stageMask, GAPI::ShaderStageMask::Vertex)) stages = stages | wgpu::ShaderStage::Vertex;
            if (Common::IsSet(stageMask, GAPI::ShaderStageMask::Pixel)) stages = stages | wgpu::ShaderStage::Fragment;
            if (Common::IsSet(stageMask, GAPI::ShaderStageMask::Compute)) stages = stages | wgpu::ShaderStage::Compute;
            return stages;
        }

        constexpr wgpu::TextureViewDimension getTextureViewDimension(GAPI::GpuResourceDimension dim)
        {
            switch (dim)
            {
                case GAPI::GpuResourceDimension::Texture1D: return wgpu::TextureViewDimension::_1D;
                case GAPI::GpuResourceDimension::Texture2D: return wgpu::TextureViewDimension::_2D;
                case GAPI::GpuResourceDimension::Texture3D: return wgpu::TextureViewDimension::_3D;
                case GAPI::GpuResourceDimension::TextureCube: return wgpu::TextureViewDimension::Cube;
                default:
                    ASSERT_MSG(false, "Unknown texture view dimension");
                    return wgpu::TextureViewDimension::Undefined;
            }
        }

        constexpr wgpu::TextureSampleType getTextureSampleType(GAPI::BindingLayoutTextureMeta::SampleType sampleType)
        {
            using Type = GAPI::BindingLayoutTextureMeta::SampleType;
            switch (sampleType)
            {
                case Type::Float: return wgpu::TextureSampleType::Float;
                case Type::Int: return wgpu::TextureSampleType::Sint;
                case Type::Uint: return wgpu::TextureSampleType::Uint;
                case Type::Depth: return wgpu::TextureSampleType::Depth;
                default:
                    ASSERT_MSG(false, "Unknown texture sample type");
                    return wgpu::TextureSampleType::Undefined;
            }
        }
    }

    BindingLayoutImpl::~BindingLayoutImpl() { }

    void BindingLayoutImpl::Init(const wgpu::Device& device, GAPI::BindingLayout& resource)
    {
        const auto& desc = resource.GetDesc();
        bindGroupLayouts.clear();

        for (const auto& group : desc.groups)
        {
            eastl::fixed_vector<wgpu::BindGroupLayoutEntry, GAPI::MAX_BINDINGS_PER_GROUP, false> entries;

            for (const auto& element : group.elements)
            {
                wgpu::BindGroupLayoutEntry entry;
                entry.setDefault();
                entry.binding = element.binding;
                entry.visibility = getShaderStage(element.stageMask);

                switch (element.type)
                {
                    case GAPI::BindingType::ConstantBuffer:
                    {
                        entry.buffer.type = wgpu::BufferBindingType::Uniform;
                        entry.buffer.hasDynamicOffset = false;
                        entry.buffer.minBindingSize = 0; // Will be set from shader reflection
                        break;
                    }
                    case GAPI::BindingType::BufferSRV:
                    {
                        entry.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
                        entry.buffer.hasDynamicOffset = false;
                        entry.buffer.minBindingSize = 0;
                        break;
                    }
                    case GAPI::BindingType::BufferUAV:
                    {
                        entry.buffer.type = wgpu::BufferBindingType::Storage;
                        entry.buffer.hasDynamicOffset = false;
                        entry.buffer.minBindingSize = 0;
                        break;
                    }
                    case GAPI::BindingType::TextureSRV:
                    {
                        entry.texture.sampleType = getTextureSampleType(desc.textureMetas[element.textureMetaIndex].sampleType);
                        entry.texture.viewDimension = getTextureViewDimension(desc.textureMetas[element.textureMetaIndex].dimension);
                        entry.texture.multisampled = false;
                        break;
                    }
                    case GAPI::BindingType::TextureUAV:
                    {
                        entry.storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
                        entry.storageTexture.format = GetWGPUFormat(desc.textureMetas[element.textureMetaIndex].format);
                        entry.storageTexture.viewDimension = getTextureViewDimension(desc.textureMetas[element.textureMetaIndex].dimension);
                        break;
                    }
                    case GAPI::BindingType::Sampler:
                    {
                        entry.sampler.type = wgpu::SamplerBindingType::Filtering;
                        break;
                    }
                    default:
                        ASSERT_MSG(false, "Unknown binding type");
                        continue;
                }

                entries.push_back(entry);
            }

            wgpu::BindGroupLayoutDescriptor layoutDesc;
            layoutDesc.setDefault();
            layoutDesc.label = wgpu::StringView(resource.GetName().c_str());
            layoutDesc.entryCount = entries.size();
            layoutDesc.entries = entries.data();

            bindGroupLayouts.push_back(device.createBindGroupLayout(layoutDesc));
        }
    }
}

