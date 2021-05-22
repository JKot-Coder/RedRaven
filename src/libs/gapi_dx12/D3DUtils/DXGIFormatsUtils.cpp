#include "DXGIFormatsUtils.hpp"

#include "gapi/GpuResource.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        namespace DX12
        {
            namespace D3DUtils
            {
                struct GpuResourceFormatConversion
                {
                    GpuResourceFormat from;
                    DXGI_FORMAT to;
                    DXGI_FORMAT typeless;
                };

                // clang-format off
                static GpuResourceFormatConversion formatsConversion[] = {
                    { GpuResourceFormat::Unknown,           DXGI_FORMAT_UNKNOWN,                    DXGI_FORMAT_UNKNOWN },
                    { GpuResourceFormat::RGBA32Float,       DXGI_FORMAT_R32G32B32A32_FLOAT,         DXGI_FORMAT_R32G32B32A32_TYPELESS },
                    { GpuResourceFormat::RGBA32Uint,        DXGI_FORMAT_R32G32B32A32_UINT,          DXGI_FORMAT_R32G32B32A32_TYPELESS },
                    { GpuResourceFormat::RGBA32Sint,        DXGI_FORMAT_R32G32B32A32_SINT,          DXGI_FORMAT_R32G32B32A32_TYPELESS },
                    { GpuResourceFormat::RGB32Float,        DXGI_FORMAT_R32G32B32_FLOAT,            DXGI_FORMAT_R32G32B32_TYPELESS },
                    { GpuResourceFormat::RGB32Uint,         DXGI_FORMAT_R32G32B32_UINT,             DXGI_FORMAT_R32G32B32_TYPELESS },
                    { GpuResourceFormat::RGB32Sint,         DXGI_FORMAT_R32G32B32_SINT,             DXGI_FORMAT_R32G32B32_TYPELESS },
                    { GpuResourceFormat::RGBA16Float,       DXGI_FORMAT_R16G16B16A16_FLOAT,         DXGI_FORMAT_R16G16B16A16_TYPELESS },
                    { GpuResourceFormat::RGBA16Unorm,       DXGI_FORMAT_R16G16B16A16_UNORM,         DXGI_FORMAT_R16G16B16A16_TYPELESS },
                    { GpuResourceFormat::RGBA16Uint,        DXGI_FORMAT_R16G16B16A16_UINT,          DXGI_FORMAT_R16G16B16A16_TYPELESS },
                    { GpuResourceFormat::RGBA16Snorm,       DXGI_FORMAT_R16G16B16A16_SNORM,         DXGI_FORMAT_R16G16B16A16_TYPELESS },
                    { GpuResourceFormat::RGBA16Sint,        DXGI_FORMAT_R16G16B16A16_SINT,          DXGI_FORMAT_R16G16B16A16_TYPELESS },
                    { GpuResourceFormat::RG32Float,         DXGI_FORMAT_R32G32_FLOAT,               DXGI_FORMAT_R32G32_TYPELESS },
                    { GpuResourceFormat::RG32Uint,          DXGI_FORMAT_R32G32_UINT,                DXGI_FORMAT_R32G32_TYPELESS },
                    { GpuResourceFormat::RG32Sint,          DXGI_FORMAT_R32G32_SINT,                DXGI_FORMAT_R32G32_TYPELESS },

                    { GpuResourceFormat::RGB10A2Unorm,      DXGI_FORMAT_R10G10B10A2_UNORM,          DXGI_FORMAT_R10G10B10A2_TYPELESS },
                    { GpuResourceFormat::RGB10A2Uint,       DXGI_FORMAT_R10G10B10A2_UINT,           DXGI_FORMAT_R10G10B10A2_TYPELESS },
                    { GpuResourceFormat::R11G11B10Float,    DXGI_FORMAT_R11G11B10_FLOAT,            DXGI_FORMAT_UNKNOWN },
                    { GpuResourceFormat::RGBA8Unorm,        DXGI_FORMAT_R8G8B8A8_UNORM,             DXGI_FORMAT_R8G8B8A8_TYPELESS },
                    { GpuResourceFormat::RGBA8UnormSrgb,    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,        DXGI_FORMAT_R8G8B8A8_TYPELESS },
                    { GpuResourceFormat::RGBA8Uint,         DXGI_FORMAT_R8G8B8A8_UINT,              DXGI_FORMAT_R8G8B8A8_TYPELESS },
                    { GpuResourceFormat::RGBA8Snorm,        DXGI_FORMAT_R8G8B8A8_SNORM,             DXGI_FORMAT_R8G8B8A8_TYPELESS },
                    { GpuResourceFormat::RGBA8Sint,         DXGI_FORMAT_R8G8B8A8_SINT,              DXGI_FORMAT_R8G8B8A8_TYPELESS },
                    { GpuResourceFormat::RG16Float,         DXGI_FORMAT_R16G16_FLOAT,               DXGI_FORMAT_R16G16_TYPELESS },
                    { GpuResourceFormat::RG16Unorm,         DXGI_FORMAT_R16G16_UNORM,               DXGI_FORMAT_R16G16_TYPELESS },
                    { GpuResourceFormat::RG16Uint,          DXGI_FORMAT_R16G16_UINT,                DXGI_FORMAT_R16G16_TYPELESS },
                    { GpuResourceFormat::RG16Snorm,         DXGI_FORMAT_R16G16_SNORM,               DXGI_FORMAT_R16G16_TYPELESS },
                    { GpuResourceFormat::RG16Sint,          DXGI_FORMAT_R16G16_SINT,                DXGI_FORMAT_R16G16_TYPELESS },

                    { GpuResourceFormat::R32Float,          DXGI_FORMAT_R32_FLOAT,                  DXGI_FORMAT_R32_TYPELESS },
                    { GpuResourceFormat::R32Uint,           DXGI_FORMAT_R32_UINT,                   DXGI_FORMAT_R32_TYPELESS },
                    { GpuResourceFormat::R32Sint,           DXGI_FORMAT_R32_SINT,                   DXGI_FORMAT_R32_TYPELESS },

                    { GpuResourceFormat::RG8Unorm,          DXGI_FORMAT_R8G8_UNORM,                 DXGI_FORMAT_R8G8_TYPELESS },
                    { GpuResourceFormat::RG8Uint,           DXGI_FORMAT_R8G8_UINT,                  DXGI_FORMAT_R8G8_TYPELESS },
                    { GpuResourceFormat::RG8Snorm,          DXGI_FORMAT_R8G8_SNORM,                 DXGI_FORMAT_R8G8_TYPELESS },
                    { GpuResourceFormat::RG8Sint,           DXGI_FORMAT_R8G8_SINT,                  DXGI_FORMAT_R8G8_TYPELESS },

                    { GpuResourceFormat::R16Float,          DXGI_FORMAT_R16_FLOAT,                  DXGI_FORMAT_R16_TYPELESS },
                    { GpuResourceFormat::R16Unorm,          DXGI_FORMAT_R16_UNORM,                  DXGI_FORMAT_R16_TYPELESS },
                    { GpuResourceFormat::R16Uint,           DXGI_FORMAT_R16_UINT,                   DXGI_FORMAT_R16_TYPELESS },
                    { GpuResourceFormat::R16Snorm,          DXGI_FORMAT_R16_SNORM,                  DXGI_FORMAT_R16_TYPELESS },
                    { GpuResourceFormat::R16Sint,           DXGI_FORMAT_R16_SINT,                   DXGI_FORMAT_R16_TYPELESS },
                    { GpuResourceFormat::R8Unorm,           DXGI_FORMAT_R8_UNORM,                   DXGI_FORMAT_R8_TYPELESS },
                    { GpuResourceFormat::R8Uint,            DXGI_FORMAT_R8_UINT,                    DXGI_FORMAT_R8_TYPELESS },
                    { GpuResourceFormat::R8Snorm,           DXGI_FORMAT_R8_SNORM,                   DXGI_FORMAT_R8_TYPELESS },
                    { GpuResourceFormat::R8Sint,            DXGI_FORMAT_R8_SINT,                    DXGI_FORMAT_R8_TYPELESS },
                    { GpuResourceFormat::A8Unorm,           DXGI_FORMAT_A8_UNORM,                   DXGI_FORMAT_R8_TYPELESS },

                    { GpuResourceFormat::D32FloatS8X24Uint, DXGI_FORMAT_D32_FLOAT_S8X24_UINT,       DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS },
                    { GpuResourceFormat::D32Float,          DXGI_FORMAT_D32_FLOAT,                  DXGI_FORMAT_R32_TYPELESS },
                    { GpuResourceFormat::D24UnormS8Uint,    DXGI_FORMAT_D24_UNORM_S8_UINT,          DXGI_FORMAT_R24G8_TYPELESS },
                    { GpuResourceFormat::D16Unorm,          DXGI_FORMAT_D16_UNORM,                  DXGI_FORMAT_R16_TYPELESS },

                    { GpuResourceFormat::R32FloatX8X24,     DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,   DXGI_FORMAT_R32G8X24_TYPELESS },
                    { GpuResourceFormat::X32G8Uint,         DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,    DXGI_FORMAT_X32_TYPELESS_G8X24_UINT },
                    { GpuResourceFormat::R24UnormX8,        DXGI_FORMAT_R24_UNORM_X8_TYPELESS,      DXGI_FORMAT_R24G8_TYPELESS },
                    { GpuResourceFormat::X24G8Uint,         DXGI_FORMAT_X24_TYPELESS_G8_UINT,       DXGI_FORMAT_X24_TYPELESS_G8_UINT },

                    { GpuResourceFormat::BC1Unorm,          DXGI_FORMAT_BC1_UNORM,                  DXGI_FORMAT_BC1_TYPELESS },
                    { GpuResourceFormat::BC1UnormSrgb,      DXGI_FORMAT_BC1_UNORM_SRGB,             DXGI_FORMAT_BC1_TYPELESS },
                    { GpuResourceFormat::BC2Unorm,          DXGI_FORMAT_BC2_UNORM,                  DXGI_FORMAT_BC2_TYPELESS },
                    { GpuResourceFormat::BC2UnormSrgb,      DXGI_FORMAT_BC2_UNORM_SRGB,             DXGI_FORMAT_BC2_TYPELESS },
                    { GpuResourceFormat::BC3Unorm,          DXGI_FORMAT_BC3_UNORM,                  DXGI_FORMAT_BC3_TYPELESS },
                    { GpuResourceFormat::BC3UnormSrgb,      DXGI_FORMAT_BC3_UNORM_SRGB,             DXGI_FORMAT_BC3_TYPELESS },
                    { GpuResourceFormat::BC4Unorm,          DXGI_FORMAT_BC4_UNORM,                  DXGI_FORMAT_BC4_TYPELESS },
                    { GpuResourceFormat::BC4Snorm,          DXGI_FORMAT_BC4_SNORM,                  DXGI_FORMAT_BC4_TYPELESS },
                    { GpuResourceFormat::BC5Unorm,          DXGI_FORMAT_BC5_UNORM,                  DXGI_FORMAT_BC5_TYPELESS },
                    { GpuResourceFormat::BC5Snorm,          DXGI_FORMAT_BC5_SNORM,                  DXGI_FORMAT_BC5_TYPELESS },
                    { GpuResourceFormat::BC6HU16,           DXGI_FORMAT_BC6H_UF16,                  DXGI_FORMAT_BC6H_TYPELESS },
                    { GpuResourceFormat::BC6HS16,           DXGI_FORMAT_BC6H_SF16,                  DXGI_FORMAT_BC6H_TYPELESS },
                    { GpuResourceFormat::BC7Unorm,          DXGI_FORMAT_BC7_UNORM,                  DXGI_FORMAT_BC7_TYPELESS },
                    { GpuResourceFormat::BC7UnormSrgb,      DXGI_FORMAT_BC7_UNORM_SRGB,             DXGI_FORMAT_BC7_TYPELESS },

                    { GpuResourceFormat::RGB16Float,        DXGI_FORMAT_UNKNOWN,                    DXGI_FORMAT_UNKNOWN },
                    { GpuResourceFormat::RGB16Unorm,        DXGI_FORMAT_UNKNOWN,                    DXGI_FORMAT_UNKNOWN },
                    { GpuResourceFormat::RGB16Uint,         DXGI_FORMAT_UNKNOWN,                    DXGI_FORMAT_UNKNOWN },
                    { GpuResourceFormat::RGB16Snorm,        DXGI_FORMAT_UNKNOWN,                    DXGI_FORMAT_UNKNOWN },
                    { GpuResourceFormat::RGB16Sint,         DXGI_FORMAT_UNKNOWN,                    DXGI_FORMAT_UNKNOWN },

                    { GpuResourceFormat::RGB5A1Unorm,       DXGI_FORMAT_B5G5R5A1_UNORM,             DXGI_FORMAT_UNKNOWN },
                    { GpuResourceFormat::RGB9E5Float,       DXGI_FORMAT_R9G9B9E5_SHAREDEXP,         DXGI_FORMAT_UNKNOWN },

                    { GpuResourceFormat::BGRA8Unorm,        DXGI_FORMAT_B8G8R8A8_UNORM,             DXGI_FORMAT_B8G8R8A8_TYPELESS },
                    { GpuResourceFormat::BGRA8UnormSrgb,    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,        DXGI_FORMAT_B8G8R8A8_TYPELESS },
                    { GpuResourceFormat::BGRX8Unorm,        DXGI_FORMAT_B8G8R8X8_UNORM,             DXGI_FORMAT_B8G8R8A8_TYPELESS },
                    { GpuResourceFormat::BGRX8UnormSrgb,    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,        DXGI_FORMAT_B8G8R8A8_TYPELESS },

                    { GpuResourceFormat::R5G6B5Unorm,       DXGI_FORMAT_B5G6R5_UNORM,               DXGI_FORMAT_UNKNOWN },
                }; // clang-format on

                static_assert(std::is_same<std::underlying_type<GpuResourceFormat>::type, uint32_t>::value);
                static_assert(std::size(formatsConversion) == static_cast<uint32_t>(GpuResourceFormat::Count));

                DXGI_FORMAT GetDxgiResourceFormat(GpuResourceFormat format)
                {
                    ASSERT(formatsConversion[static_cast<uint32_t>(format)].from == format);
                    ASSERT(format == GpuResourceFormat::Unknown ||
                           formatsConversion[static_cast<uint32_t>(format)].to != DXGI_FORMAT_UNKNOWN);

                    return formatsConversion[static_cast<uint32_t>(format)].to;
                }

                DXGI_FORMAT GetDxgiTypelessFormat(GpuResourceFormat format)
                {
                    ASSERT(formatsConversion[static_cast<uint32_t>(format)].from == format);
                    ASSERT(format == GpuResourceFormat::Unknown ||
                           formatsConversion[static_cast<uint32_t>(format)].typeless != DXGI_FORMAT_UNKNOWN);

                    return formatsConversion[static_cast<uint32_t>(format)].typeless;
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
            }
        }
    }
}