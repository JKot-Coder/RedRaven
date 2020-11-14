#include "TypeConversions.hpp"

namespace OpenDemo
{
    namespace Render
    {
        namespace DX12
        {
            namespace TypeConversions
            {
                struct ResourceFormatConversion
                {
                    Resource::Format::Value from;
                    ::DXGI_FORMAT to;
                };

                // clang-format off
                static ResourceFormatConversion formatsConversion[] = {
                    { Resource::Format::Unknown,           DXGI_FORMAT_UNKNOWN },
                    { Resource::Format::RGBA32Float,       DXGI_FORMAT_R32G32B32A32_FLOAT },
                    { Resource::Format::RGBA32Uint,        DXGI_FORMAT_R32G32B32A32_UINT },
                    { Resource::Format::RGBA32Sint,        DXGI_FORMAT_R32G32B32A32_SINT },
                    { Resource::Format::RGB32Float,        DXGI_FORMAT_R32G32B32_FLOAT },
                    { Resource::Format::RGB32Uint,         DXGI_FORMAT_R32G32B32_UINT },
                    { Resource::Format::RGB32Sint,         DXGI_FORMAT_R32G32B32_SINT },
                    { Resource::Format::RGBA16Float,       DXGI_FORMAT_R16G16B16A16_FLOAT },
                    { Resource::Format::RGBA16Unorm,       DXGI_FORMAT_R16G16B16A16_UNORM },
                    { Resource::Format::RGBA16Uint,        DXGI_FORMAT_R16G16B16A16_UINT },
                    { Resource::Format::RGBA16Snorm,       DXGI_FORMAT_R16G16B16A16_SNORM },
                    { Resource::Format::RGBA16Sint,        DXGI_FORMAT_R16G16B16A16_SINT },
                    { Resource::Format::RG32Float,         DXGI_FORMAT_R32G32_FLOAT },
                    { Resource::Format::RG32Uint,          DXGI_FORMAT_R32G32_UINT },
                    { Resource::Format::RG32Sint,          DXGI_FORMAT_R32G32_SINT },

                    { Resource::Format::RGB10A2Unorm,      DXGI_FORMAT_R10G10B10A2_UNORM },
                    { Resource::Format::RGB10A2Uint,       DXGI_FORMAT_R10G10B10A2_UINT },
                    { Resource::Format::R11G11B10Float,    DXGI_FORMAT_R11G11B10_FLOAT },
                    { Resource::Format::RGBA8Unorm,        DXGI_FORMAT_R8G8B8A8_UNORM },
                    { Resource::Format::RGBA8UnormSrgb,    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB },
                    { Resource::Format::RGBA8Uint,         DXGI_FORMAT_R8G8B8A8_UINT },
                    { Resource::Format::RGBA8Snorm,        DXGI_FORMAT_R8G8B8A8_SNORM },
                    { Resource::Format::RGBA8Sint,         DXGI_FORMAT_R8G8B8A8_SINT },
                    { Resource::Format::RG16Float,         DXGI_FORMAT_R16G16_FLOAT },
                    { Resource::Format::RG16Unorm,         DXGI_FORMAT_R16G16_UNORM },
                    { Resource::Format::RG16Uint,          DXGI_FORMAT_R16G16_UINT },
                    { Resource::Format::RG16Snorm,         DXGI_FORMAT_R16G16_SNORM },
                    { Resource::Format::RG16Sint,          DXGI_FORMAT_R16G16_SINT },

                    { Resource::Format::R32Float,          DXGI_FORMAT_R32_FLOAT },
                    { Resource::Format::R32Uint,           DXGI_FORMAT_R32_UINT },
                    { Resource::Format::R32Sint,           DXGI_FORMAT_R32_SINT },

                    { Resource::Format::RG8Unorm,          DXGI_FORMAT_R8G8_UNORM },
                    { Resource::Format::RG8Uint,           DXGI_FORMAT_R8G8_UINT },
                    { Resource::Format::RG8Snorm,          DXGI_FORMAT_R8G8_SNORM },
                    { Resource::Format::RG8Sint,           DXGI_FORMAT_R8G8_SINT },

                    { Resource::Format::R16Float,          DXGI_FORMAT_R16_FLOAT },
                    { Resource::Format::R16Unorm,          DXGI_FORMAT_R16_UNORM },
                    { Resource::Format::R16Uint,           DXGI_FORMAT_R16_UINT },
                    { Resource::Format::R16Snorm,          DXGI_FORMAT_R16_SNORM },
                    { Resource::Format::R16Sint,           DXGI_FORMAT_R16_SINT },
                    { Resource::Format::R8Unorm,           DXGI_FORMAT_R8_UNORM },
                    { Resource::Format::R8Uint,            DXGI_FORMAT_R8_UINT },
                    { Resource::Format::R8Snorm,           DXGI_FORMAT_R8_SNORM },
                    { Resource::Format::R8Sint,            DXGI_FORMAT_R8_SINT },
                    { Resource::Format::A8Unorm,           DXGI_FORMAT_A8_UNORM },

                    { Resource::Format::D32FloatS8X24Uint, DXGI_FORMAT_D32_FLOAT_S8X24_UINT },
                    { Resource::Format::D32Float,          DXGI_FORMAT_D32_FLOAT },
                    { Resource::Format::D24UnormS8Uint,    DXGI_FORMAT_D24_UNORM_S8_UINT },
                    { Resource::Format::D16Unorm,          DXGI_FORMAT_D16_UNORM },

                    { Resource::Format::R32FloatX8X24,     DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS },
                    { Resource::Format::X32G8Uint,         DXGI_FORMAT_X32_TYPELESS_G8X24_UINT },
                    { Resource::Format::R24UnormX8,        DXGI_FORMAT_R24_UNORM_X8_TYPELESS },
                    { Resource::Format::X24G8Uint,         DXGI_FORMAT_X24_TYPELESS_G8_UINT },

                    { Resource::Format::BC1Unorm,          DXGI_FORMAT_BC1_UNORM },
                    { Resource::Format::BC1UnormSrgb,      DXGI_FORMAT_BC1_UNORM_SRGB },
                    { Resource::Format::BC2Unorm,          DXGI_FORMAT_BC2_UNORM },
                    { Resource::Format::BC2UnormSrgb,      DXGI_FORMAT_BC2_UNORM_SRGB },
                    { Resource::Format::BC3Unorm,          DXGI_FORMAT_BC3_UNORM },
                    { Resource::Format::BC3UnormSrgb,      DXGI_FORMAT_BC3_UNORM_SRGB },
                    { Resource::Format::BC4Unorm,          DXGI_FORMAT_BC4_UNORM },
                    { Resource::Format::BC4Snorm,          DXGI_FORMAT_BC4_SNORM },
                    { Resource::Format::BC5Unorm,          DXGI_FORMAT_BC5_UNORM },
                    { Resource::Format::BC5Snorm,          DXGI_FORMAT_BC5_SNORM },
                    { Resource::Format::BC6HU16,           DXGI_FORMAT_BC6H_UF16 },
                    { Resource::Format::BC6HS16,           DXGI_FORMAT_BC6H_SF16 },
                    { Resource::Format::BC7Unorm,          DXGI_FORMAT_BC7_UNORM },
                    { Resource::Format::BC7UnormSrgb,      DXGI_FORMAT_BC7_UNORM_SRGB },

                    { Resource::Format::RGB16Float,        DXGI_FORMAT_UNKNOWN },
                    { Resource::Format::RGB16Unorm,        DXGI_FORMAT_UNKNOWN },
                    { Resource::Format::RGB16Uint,         DXGI_FORMAT_UNKNOWN },
                    { Resource::Format::RGB16Snorm,        DXGI_FORMAT_UNKNOWN },
                    { Resource::Format::RGB16Sint,         DXGI_FORMAT_UNKNOWN },

                    { Resource::Format::RGB5A1Unorm,       DXGI_FORMAT_B5G5R5A1_UNORM },
                    { Resource::Format::RGB9E5Float,       DXGI_FORMAT_R9G9B9E5_SHAREDEXP },

                    { Resource::Format::BGRA8Unorm,        DXGI_FORMAT_B8G8R8A8_UNORM },
                    { Resource::Format::BGRA8UnormSrgb,    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB },
                    { Resource::Format::BGRX8Unorm,        DXGI_FORMAT_B8G8R8X8_UNORM },
                    { Resource::Format::BGRX8UnormSrgb,    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB },

                    { Resource::Format::R5G6B5Unorm,       DXGI_FORMAT_B5G6R5_UNORM },
                    { Resource::Format::Alpha32Float,      DXGI_FORMAT_UNKNOWN },
                }; // clang-format on

                static_assert(std::size(formatsConversion) == Resource::Format::Count);

                ::DXGI_FORMAT ResourceFormat(Resource::Format format)
                {
                    ASSERT(format.IsValid())
                    ASSERT(formatsConversion[format].from == format)
                    ASSERT(formatsConversion[format].to != DXGI_FORMAT_UNKNOWN)
                    return formatsConversion[format].to;
                }

                D3D12_RESOURCE_FLAGS ResourceFlags(Resource::BindFlags flags)
                {
                    D3D12_RESOURCE_FLAGS d3d = D3D12_RESOURCE_FLAG_NONE;

                    d3d |= IsSet(flags, Resource::BindFlags::UnorderedAccess) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
                    d3d |= IsSet(flags, Resource::BindFlags::ShaderResource) ? D3D12_RESOURCE_FLAG_NONE : D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
                    d3d |= IsSet(flags, Resource::BindFlags::DepthStencil) ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_NONE;
                    d3d |= IsSet(flags, Resource::BindFlags::RenderTarget) ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET : D3D12_RESOURCE_FLAG_NONE;

                    return d3d;
                }
            }
        }
    }
}