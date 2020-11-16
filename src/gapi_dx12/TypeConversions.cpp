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
                    ResourceFormat from;
                    ::DXGI_FORMAT to;
                };

                // clang-format off
                static ResourceFormatConversion formatsConversion[] = {
                    { ResourceFormat::Unknown,           DXGI_FORMAT_UNKNOWN },
                    { ResourceFormat::RGBA32Float,       DXGI_FORMAT_R32G32B32A32_FLOAT },
                    { ResourceFormat::RGBA32Uint,        DXGI_FORMAT_R32G32B32A32_UINT },
                    { ResourceFormat::RGBA32Sint,        DXGI_FORMAT_R32G32B32A32_SINT },
                    { ResourceFormat::RGB32Float,        DXGI_FORMAT_R32G32B32_FLOAT },
                    { ResourceFormat::RGB32Uint,         DXGI_FORMAT_R32G32B32_UINT },
                    { ResourceFormat::RGB32Sint,         DXGI_FORMAT_R32G32B32_SINT },
                    { ResourceFormat::RGBA16Float,       DXGI_FORMAT_R16G16B16A16_FLOAT },
                    { ResourceFormat::RGBA16Unorm,       DXGI_FORMAT_R16G16B16A16_UNORM },
                    { ResourceFormat::RGBA16Uint,        DXGI_FORMAT_R16G16B16A16_UINT },
                    { ResourceFormat::RGBA16Snorm,       DXGI_FORMAT_R16G16B16A16_SNORM },
                    { ResourceFormat::RGBA16Sint,        DXGI_FORMAT_R16G16B16A16_SINT },
                    { ResourceFormat::RG32Float,         DXGI_FORMAT_R32G32_FLOAT },
                    { ResourceFormat::RG32Uint,          DXGI_FORMAT_R32G32_UINT },
                    { ResourceFormat::RG32Sint,          DXGI_FORMAT_R32G32_SINT },

                    { ResourceFormat::RGB10A2Unorm,      DXGI_FORMAT_R10G10B10A2_UNORM },
                    { ResourceFormat::RGB10A2Uint,       DXGI_FORMAT_R10G10B10A2_UINT },
                    { ResourceFormat::R11G11B10Float,    DXGI_FORMAT_R11G11B10_FLOAT },
                    { ResourceFormat::RGBA8Unorm,        DXGI_FORMAT_R8G8B8A8_UNORM },
                    { ResourceFormat::RGBA8UnormSrgb,    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB },
                    { ResourceFormat::RGBA8Uint,         DXGI_FORMAT_R8G8B8A8_UINT },
                    { ResourceFormat::RGBA8Snorm,        DXGI_FORMAT_R8G8B8A8_SNORM },
                    { ResourceFormat::RGBA8Sint,         DXGI_FORMAT_R8G8B8A8_SINT },
                    { ResourceFormat::RG16Float,         DXGI_FORMAT_R16G16_FLOAT },
                    { ResourceFormat::RG16Unorm,         DXGI_FORMAT_R16G16_UNORM },
                    { ResourceFormat::RG16Uint,          DXGI_FORMAT_R16G16_UINT },
                    { ResourceFormat::RG16Snorm,         DXGI_FORMAT_R16G16_SNORM },
                    { ResourceFormat::RG16Sint,          DXGI_FORMAT_R16G16_SINT },

                    { ResourceFormat::R32Float,          DXGI_FORMAT_R32_FLOAT },
                    { ResourceFormat::R32Uint,           DXGI_FORMAT_R32_UINT },
                    { ResourceFormat::R32Sint,           DXGI_FORMAT_R32_SINT },

                    { ResourceFormat::RG8Unorm,          DXGI_FORMAT_R8G8_UNORM },
                    { ResourceFormat::RG8Uint,           DXGI_FORMAT_R8G8_UINT },
                    { ResourceFormat::RG8Snorm,          DXGI_FORMAT_R8G8_SNORM },
                    { ResourceFormat::RG8Sint,           DXGI_FORMAT_R8G8_SINT },

                    { ResourceFormat::R16Float,          DXGI_FORMAT_R16_FLOAT },
                    { ResourceFormat::R16Unorm,          DXGI_FORMAT_R16_UNORM },
                    { ResourceFormat::R16Uint,           DXGI_FORMAT_R16_UINT },
                    { ResourceFormat::R16Snorm,          DXGI_FORMAT_R16_SNORM },
                    { ResourceFormat::R16Sint,           DXGI_FORMAT_R16_SINT },
                    { ResourceFormat::R8Unorm,           DXGI_FORMAT_R8_UNORM },
                    { ResourceFormat::R8Uint,            DXGI_FORMAT_R8_UINT },
                    { ResourceFormat::R8Snorm,           DXGI_FORMAT_R8_SNORM },
                    { ResourceFormat::R8Sint,            DXGI_FORMAT_R8_SINT },
                    { ResourceFormat::A8Unorm,           DXGI_FORMAT_A8_UNORM },

                    { ResourceFormat::D32FloatS8X24Uint, DXGI_FORMAT_D32_FLOAT_S8X24_UINT },
                    { ResourceFormat::D32Float,          DXGI_FORMAT_D32_FLOAT },
                    { ResourceFormat::D24UnormS8Uint,    DXGI_FORMAT_D24_UNORM_S8_UINT },
                    { ResourceFormat::D16Unorm,          DXGI_FORMAT_D16_UNORM },

                    { ResourceFormat::R32FloatX8X24,     DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS },
                    { ResourceFormat::X32G8Uint,         DXGI_FORMAT_X32_TYPELESS_G8X24_UINT },
                    { ResourceFormat::R24UnormX8,        DXGI_FORMAT_R24_UNORM_X8_TYPELESS },
                    { ResourceFormat::X24G8Uint,         DXGI_FORMAT_X24_TYPELESS_G8_UINT },

                    { ResourceFormat::BC1Unorm,          DXGI_FORMAT_BC1_UNORM },
                    { ResourceFormat::BC1UnormSrgb,      DXGI_FORMAT_BC1_UNORM_SRGB },
                    { ResourceFormat::BC2Unorm,          DXGI_FORMAT_BC2_UNORM },
                    { ResourceFormat::BC2UnormSrgb,      DXGI_FORMAT_BC2_UNORM_SRGB },
                    { ResourceFormat::BC3Unorm,          DXGI_FORMAT_BC3_UNORM },
                    { ResourceFormat::BC3UnormSrgb,      DXGI_FORMAT_BC3_UNORM_SRGB },
                    { ResourceFormat::BC4Unorm,          DXGI_FORMAT_BC4_UNORM },
                    { ResourceFormat::BC4Snorm,          DXGI_FORMAT_BC4_SNORM },
                    { ResourceFormat::BC5Unorm,          DXGI_FORMAT_BC5_UNORM },
                    { ResourceFormat::BC5Snorm,          DXGI_FORMAT_BC5_SNORM },
                    { ResourceFormat::BC6HU16,           DXGI_FORMAT_BC6H_UF16 },
                    { ResourceFormat::BC6HS16,           DXGI_FORMAT_BC6H_SF16 },
                    { ResourceFormat::BC7Unorm,          DXGI_FORMAT_BC7_UNORM },
                    { ResourceFormat::BC7UnormSrgb,      DXGI_FORMAT_BC7_UNORM_SRGB },

                    { ResourceFormat::RGB16Float,        DXGI_FORMAT_UNKNOWN },
                    { ResourceFormat::RGB16Unorm,        DXGI_FORMAT_UNKNOWN },
                    { ResourceFormat::RGB16Uint,         DXGI_FORMAT_UNKNOWN },
                    { ResourceFormat::RGB16Snorm,        DXGI_FORMAT_UNKNOWN },
                    { ResourceFormat::RGB16Sint,         DXGI_FORMAT_UNKNOWN },

                    { ResourceFormat::RGB5A1Unorm,       DXGI_FORMAT_B5G5R5A1_UNORM },
                    { ResourceFormat::RGB9E5Float,       DXGI_FORMAT_R9G9B9E5_SHAREDEXP },

                    { ResourceFormat::BGRA8Unorm,        DXGI_FORMAT_B8G8R8A8_UNORM },
                    { ResourceFormat::BGRA8UnormSrgb,    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB },
                    { ResourceFormat::BGRX8Unorm,        DXGI_FORMAT_B8G8R8X8_UNORM },
                    { ResourceFormat::BGRX8UnormSrgb,    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB },

                    { ResourceFormat::R5G6B5Unorm,       DXGI_FORMAT_B5G6R5_UNORM },
                    { ResourceFormat::Alpha32Float,      DXGI_FORMAT_UNKNOWN },
                }; // clang-format on

                static_assert(std::is_same<std::underlying_type<ResourceFormat>::type, uint32_t>::value);
                static_assert(std::size(formatsConversion) == static_cast<uint32_t>(ResourceFormat::Count));

                ::DXGI_FORMAT GetResourceFormat(ResourceFormat format)
                {
                    ASSERT(formatsConversion[static_cast<uint32_t>(format)].from == format)
                    ASSERT(formatsConversion[static_cast<uint32_t>(format)].to != DXGI_FORMAT_UNKNOWN)
                    return formatsConversion[static_cast<uint32_t>(format)].to;
                }

                D3D12_RESOURCE_FLAGS GetResourceFlags(Resource::BindFlags flags)
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