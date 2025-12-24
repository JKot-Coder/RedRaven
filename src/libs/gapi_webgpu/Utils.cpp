#include "Utils.hpp"

#include "gapi/GpuResource.hpp"

namespace RR::GAPI::WebGPU
{
    wgpu::TextureFormat GetWGPUFormat(GpuResourceFormat format)
    {
        switch (format)
        {
            case GpuResourceFormat::RGBA32Float: return wgpu::TextureFormat::RGBA32Float;
            case GpuResourceFormat::RGBA32Uint: return wgpu::TextureFormat::RGBA32Uint;
            case GpuResourceFormat::RGBA32Sint: return wgpu::TextureFormat::RGBA32Sint;
            case GpuResourceFormat::RGBA16Float: return wgpu::TextureFormat::RGBA16Float;
            case GpuResourceFormat::RGBA16Unorm: return wgpu::TextureFormat::RGBA16Unorm;
            case GpuResourceFormat::RGBA16Uint: return wgpu::TextureFormat::RGBA16Uint;
            case GpuResourceFormat::RGBA16Snorm: return wgpu::TextureFormat::RGBA16Snorm;
            case GpuResourceFormat::RGBA16Sint: return wgpu::TextureFormat::RGBA16Sint;
            case GpuResourceFormat::RG32Float: return wgpu::TextureFormat::RG32Float;
            case GpuResourceFormat::RG32Uint: return wgpu::TextureFormat::RG32Uint;
            case GpuResourceFormat::RG32Sint: return wgpu::TextureFormat::RG32Sint;

            case GpuResourceFormat::RGB10A2Unorm: return wgpu::TextureFormat::RGB10A2Unorm;
            case GpuResourceFormat::RGB10A2Uint: return wgpu::TextureFormat::RGB10A2Uint;
            case GpuResourceFormat::R11G11B10Float: return wgpu::TextureFormat::RG11B10Ufloat;
            case GpuResourceFormat::RGBA8Unorm: return wgpu::TextureFormat::RGBA8Unorm;
            case GpuResourceFormat::RGBA8UnormSrgb: return wgpu::TextureFormat::RGBA8UnormSrgb;
            case GpuResourceFormat::RGBA8Uint: return wgpu::TextureFormat::RGBA8Uint;
            case GpuResourceFormat::RGBA8Snorm: return wgpu::TextureFormat::RGBA8Snorm;
            case GpuResourceFormat::RGBA8Sint: return wgpu::TextureFormat::RGBA8Sint;
            case GpuResourceFormat::RG16Float: return wgpu::TextureFormat::RG16Float;
            case GpuResourceFormat::RG16Unorm: return wgpu::TextureFormat::RG16Unorm;
            case GpuResourceFormat::RG16Uint: return wgpu::TextureFormat::RG16Uint;
            case GpuResourceFormat::RG16Snorm: return wgpu::TextureFormat::RG16Snorm;
            case GpuResourceFormat::RG16Sint: return wgpu::TextureFormat::RG16Sint;

            case GpuResourceFormat::R32Float: return wgpu::TextureFormat::R32Float;
            case GpuResourceFormat::R32Uint: return wgpu::TextureFormat::R32Uint;
            case GpuResourceFormat::R32Sint: return wgpu::TextureFormat::R32Sint;

            case GpuResourceFormat::RG8Unorm: return wgpu::TextureFormat::RG8Unorm;
            case GpuResourceFormat::RG8Uint: return wgpu::TextureFormat::RG8Uint;
            case GpuResourceFormat::RG8Snorm: return wgpu::TextureFormat::RG8Snorm;
            case GpuResourceFormat::RG8Sint: return wgpu::TextureFormat::RG8Sint;

            case GpuResourceFormat::R16Float: return wgpu::TextureFormat::R16Float;
            case GpuResourceFormat::R16Unorm: return wgpu::TextureFormat::R16Unorm;
            case GpuResourceFormat::R16Uint: return wgpu::TextureFormat::R16Uint;
            case GpuResourceFormat::R16Snorm: return wgpu::TextureFormat::R16Snorm;
            case GpuResourceFormat::R16Sint: return wgpu::TextureFormat::R16Sint;
            case GpuResourceFormat::R8Unorm: return wgpu::TextureFormat::R8Unorm;
            case GpuResourceFormat::R8Uint: return wgpu::TextureFormat::R8Uint;
            case GpuResourceFormat::R8Snorm: return wgpu::TextureFormat::R8Snorm;
            case GpuResourceFormat::R8Sint: return wgpu::TextureFormat::R8Sint;

            case GpuResourceFormat::D32FloatS8X24Uint: return wgpu::TextureFormat::Depth32FloatStencil8;
            case GpuResourceFormat::D32Float: return wgpu::TextureFormat::Depth32Float;
            case GpuResourceFormat::D24UnormS8Uint: return wgpu::TextureFormat::Depth24PlusStencil8;
            case GpuResourceFormat::D16Unorm: return wgpu::TextureFormat::Depth16Unorm;

            case GpuResourceFormat::BC1Unorm: return wgpu::TextureFormat::BC1RGBAUnorm;
            case GpuResourceFormat::BC1UnormSrgb: return wgpu::TextureFormat::BC1RGBAUnormSrgb;
            case GpuResourceFormat::BC2Unorm: return wgpu::TextureFormat::BC2RGBAUnorm;
            case GpuResourceFormat::BC2UnormSrgb: return wgpu::TextureFormat::BC2RGBAUnormSrgb;
            case GpuResourceFormat::BC3Unorm: return wgpu::TextureFormat::BC3RGBAUnorm;
            case GpuResourceFormat::BC3UnormSrgb: return wgpu::TextureFormat::BC3RGBAUnormSrgb;
            case GpuResourceFormat::BC4Unorm: return wgpu::TextureFormat::BC4RUnorm;
            case GpuResourceFormat::BC4Snorm: return wgpu::TextureFormat::BC4RSnorm;
            case GpuResourceFormat::BC5Unorm: return wgpu::TextureFormat::BC5RGUnorm;
            case GpuResourceFormat::BC5Snorm: return wgpu::TextureFormat::BC5RGSnorm;
            case GpuResourceFormat::BC6HU16: return wgpu::TextureFormat::BC6HRGBUfloat;
            case GpuResourceFormat::BC6HS16: return wgpu::TextureFormat::BC6HRGBFloat;
            case GpuResourceFormat::BC7Unorm: return wgpu::TextureFormat::BC7RGBAUnorm;
            case GpuResourceFormat::BC7UnormSrgb: return wgpu::TextureFormat::BC7RGBAUnormSrgb;

            case GpuResourceFormat::RGB9E5Float: return wgpu::TextureFormat::RGB9E5Ufloat;

            case GpuResourceFormat::BGRA8Unorm: return wgpu::TextureFormat::BGRA8Unorm;
            case GpuResourceFormat::BGRA8UnormSrgb: return wgpu::TextureFormat::BGRA8UnormSrgb;

            default:
                ASSERT_MSG(false, "Invalid format");
                return wgpu::TextureFormat::Undefined;
        }
    }
}