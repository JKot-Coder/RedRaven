#include "Utils.hpp"

namespace DL = ::Diligent;

namespace RR::GAPI::Diligent
{
    struct GpuResourceFormatConversion
    {
        GpuResourceFormat from;
        DL::TEXTURE_FORMAT to;
        DL::TEXTURE_FORMAT typeless;
    };

    // clang-format off
    static GpuResourceFormatConversion formatsConversion[] = {
        { GpuResourceFormat::Unknown,           DL::TEX_FORMAT_UNKNOWN,                    DL::TEX_FORMAT_UNKNOWN },
        { GpuResourceFormat::RGBA32Float,       DL::TEX_FORMAT_RGBA32_FLOAT,               DL::TEX_FORMAT_RGBA32_TYPELESS },
        { GpuResourceFormat::RGBA32Uint,        DL::TEX_FORMAT_RGBA32_UINT,                DL::TEX_FORMAT_RGBA32_TYPELESS },
        { GpuResourceFormat::RGBA32Sint,        DL::TEX_FORMAT_RGBA32_SINT,                DL::TEX_FORMAT_RGBA32_TYPELESS },
        { GpuResourceFormat::RGB32Float,        DL::TEX_FORMAT_RGB32_FLOAT,                DL::TEX_FORMAT_RGB32_TYPELESS },
        { GpuResourceFormat::RGB32Uint,         DL::TEX_FORMAT_RGB32_UINT,                 DL::TEX_FORMAT_RGB32_TYPELESS },
        { GpuResourceFormat::RGB32Sint,         DL::TEX_FORMAT_RGB32_SINT,                 DL::TEX_FORMAT_RGB32_TYPELESS },
        { GpuResourceFormat::RGBA16Float,       DL::TEX_FORMAT_RGBA16_FLOAT,               DL::TEX_FORMAT_RGBA16_TYPELESS },
        { GpuResourceFormat::RGBA16Unorm,       DL::TEX_FORMAT_RGBA16_UNORM,               DL::TEX_FORMAT_RGBA16_TYPELESS },
        { GpuResourceFormat::RGBA16Uint,        DL::TEX_FORMAT_RGBA16_UINT,                DL::TEX_FORMAT_RGBA16_TYPELESS },
        { GpuResourceFormat::RGBA16Snorm,       DL::TEX_FORMAT_RGBA16_SNORM,               DL::TEX_FORMAT_RGBA16_TYPELESS },
        { GpuResourceFormat::RGBA16Sint,        DL::TEX_FORMAT_RGBA16_SINT,                DL::TEX_FORMAT_RGBA16_TYPELESS },
        { GpuResourceFormat::RG32Float,         DL::TEX_FORMAT_RG32_FLOAT,                 DL::TEX_FORMAT_RG32_TYPELESS },
        { GpuResourceFormat::RG32Uint,          DL::TEX_FORMAT_RG32_UINT,                  DL::TEX_FORMAT_RG32_TYPELESS },
        { GpuResourceFormat::RG32Sint,          DL::TEX_FORMAT_RG32_SINT,                  DL::TEX_FORMAT_RG32_TYPELESS },

        { GpuResourceFormat::RGB10A2Unorm,      DL::TEX_FORMAT_RGB10A2_UNORM,              DL::TEX_FORMAT_RGB10A2_TYPELESS },
        { GpuResourceFormat::RGB10A2Uint,       DL::TEX_FORMAT_RGB10A2_UINT,               DL::TEX_FORMAT_RGB10A2_TYPELESS },
        { GpuResourceFormat::R11G11B10Float,    DL::TEX_FORMAT_R11G11B10_FLOAT,            DL::TEX_FORMAT_UNKNOWN },
        { GpuResourceFormat::RGBA8Unorm,        DL::TEX_FORMAT_RGBA8_UNORM,                DL::TEX_FORMAT_RGBA8_TYPELESS },
        { GpuResourceFormat::RGBA8UnormSrgb,    DL::TEX_FORMAT_RGBA8_UNORM_SRGB,           DL::TEX_FORMAT_RGBA8_TYPELESS },
        { GpuResourceFormat::RGBA8Uint,         DL::TEX_FORMAT_RGBA8_UINT,                 DL::TEX_FORMAT_RGBA8_TYPELESS },
        { GpuResourceFormat::RGBA8Snorm,        DL::TEX_FORMAT_RGBA8_SNORM,                DL::TEX_FORMAT_RGBA8_TYPELESS },
        { GpuResourceFormat::RGBA8Sint,         DL::TEX_FORMAT_RGBA8_SINT,                 DL::TEX_FORMAT_RGBA8_TYPELESS },
        { GpuResourceFormat::RG16Float,         DL::TEX_FORMAT_RG16_FLOAT,                 DL::TEX_FORMAT_RG16_TYPELESS },
        { GpuResourceFormat::RG16Unorm,         DL::TEX_FORMAT_RG16_UNORM,                 DL::TEX_FORMAT_RG16_TYPELESS },
        { GpuResourceFormat::RG16Uint,          DL::TEX_FORMAT_RG16_UINT,                  DL::TEX_FORMAT_RG16_TYPELESS },
        { GpuResourceFormat::RG16Snorm,         DL::TEX_FORMAT_RG16_SNORM,                 DL::TEX_FORMAT_RG16_TYPELESS },
        { GpuResourceFormat::RG16Sint,          DL::TEX_FORMAT_RG16_SINT,                  DL::TEX_FORMAT_RG16_TYPELESS },

        { GpuResourceFormat::R32Float,          DL::TEX_FORMAT_R32_FLOAT,                  DL::TEX_FORMAT_R32_TYPELESS },
        { GpuResourceFormat::R32Uint,           DL::TEX_FORMAT_R32_UINT,                   DL::TEX_FORMAT_R32_TYPELESS },
        { GpuResourceFormat::R32Sint,           DL::TEX_FORMAT_R32_SINT,                   DL::TEX_FORMAT_R32_TYPELESS },

        { GpuResourceFormat::RG8Unorm,          DL::TEX_FORMAT_RG8_UNORM,                  DL::TEX_FORMAT_RG8_TYPELESS },
        { GpuResourceFormat::RG8Uint,           DL::TEX_FORMAT_RG8_UINT,                   DL::TEX_FORMAT_RG8_TYPELESS },
        { GpuResourceFormat::RG8Snorm,          DL::TEX_FORMAT_RG8_SNORM,                  DL::TEX_FORMAT_RG8_TYPELESS },
        { GpuResourceFormat::RG8Sint,           DL::TEX_FORMAT_RG8_SINT,                   DL::TEX_FORMAT_RG8_TYPELESS },

        { GpuResourceFormat::R16Float,          DL::TEX_FORMAT_R16_FLOAT,                  DL::TEX_FORMAT_R16_TYPELESS },
        { GpuResourceFormat::R16Unorm,          DL::TEX_FORMAT_R16_UNORM,                  DL::TEX_FORMAT_R16_TYPELESS },
        { GpuResourceFormat::R16Uint,           DL::TEX_FORMAT_R16_UINT,                   DL::TEX_FORMAT_R16_TYPELESS },
        { GpuResourceFormat::R16Snorm,          DL::TEX_FORMAT_R16_SNORM,                  DL::TEX_FORMAT_R16_TYPELESS },
        { GpuResourceFormat::R16Sint,           DL::TEX_FORMAT_R16_SINT,                   DL::TEX_FORMAT_R16_TYPELESS },
        { GpuResourceFormat::R8Unorm,           DL::TEX_FORMAT_R8_UNORM,                   DL::TEX_FORMAT_R8_TYPELESS },
        { GpuResourceFormat::R8Uint,            DL::TEX_FORMAT_R8_UINT,                    DL::TEX_FORMAT_R8_TYPELESS },
        { GpuResourceFormat::R8Snorm,           DL::TEX_FORMAT_R8_SNORM,                   DL::TEX_FORMAT_R8_TYPELESS },
        { GpuResourceFormat::R8Sint,            DL::TEX_FORMAT_R8_SINT,                    DL::TEX_FORMAT_R8_TYPELESS },
        { GpuResourceFormat::A8Unorm,           DL::TEX_FORMAT_A8_UNORM,                   DL::TEX_FORMAT_R8_TYPELESS },

        { GpuResourceFormat::D32FloatS8X24Uint, DL::TEX_FORMAT_D32_FLOAT_S8X24_UINT,       DL::TEX_FORMAT_R32_FLOAT_X8X24_TYPELESS },
        { GpuResourceFormat::D32Float,          DL::TEX_FORMAT_D32_FLOAT,                  DL::TEX_FORMAT_R32_TYPELESS },
        { GpuResourceFormat::D24UnormS8Uint,    DL::TEX_FORMAT_D24_UNORM_S8_UINT,          DL::TEX_FORMAT_R24G8_TYPELESS },
        { GpuResourceFormat::D16Unorm,          DL::TEX_FORMAT_D16_UNORM,                  DL::TEX_FORMAT_R16_TYPELESS },

        { GpuResourceFormat::R32FloatX8X24,     DL::TEX_FORMAT_R32_FLOAT_X8X24_TYPELESS,   DL::TEX_FORMAT_R32G8X24_TYPELESS },
        { GpuResourceFormat::X32G8Uint,         DL::TEX_FORMAT_X32_TYPELESS_G8X24_UINT,    DL::TEX_FORMAT_X32_TYPELESS_G8X24_UINT },
        { GpuResourceFormat::R24UnormX8,        DL::TEX_FORMAT_R24_UNORM_X8_TYPELESS,      DL::TEX_FORMAT_R24G8_TYPELESS },
        { GpuResourceFormat::X24G8Uint,         DL::TEX_FORMAT_X24_TYPELESS_G8_UINT,       DL::TEX_FORMAT_X24_TYPELESS_G8_UINT },

        { GpuResourceFormat::BC1Unorm,          DL::TEX_FORMAT_BC1_UNORM,                  DL::TEX_FORMAT_BC1_TYPELESS },
        { GpuResourceFormat::BC1UnormSrgb,      DL::TEX_FORMAT_BC1_UNORM_SRGB,             DL::TEX_FORMAT_BC1_TYPELESS },
        { GpuResourceFormat::BC2Unorm,          DL::TEX_FORMAT_BC2_UNORM,                  DL::TEX_FORMAT_BC2_TYPELESS },
        { GpuResourceFormat::BC2UnormSrgb,      DL::TEX_FORMAT_BC2_UNORM_SRGB,             DL::TEX_FORMAT_BC2_TYPELESS },
        { GpuResourceFormat::BC3Unorm,          DL::TEX_FORMAT_BC3_UNORM,                  DL::TEX_FORMAT_BC3_TYPELESS },
        { GpuResourceFormat::BC3UnormSrgb,      DL::TEX_FORMAT_BC3_UNORM_SRGB,             DL::TEX_FORMAT_BC3_TYPELESS },
        { GpuResourceFormat::BC4Unorm,          DL::TEX_FORMAT_BC4_UNORM,                  DL::TEX_FORMAT_BC4_TYPELESS },
        { GpuResourceFormat::BC4Snorm,          DL::TEX_FORMAT_BC4_SNORM,                  DL::TEX_FORMAT_BC4_TYPELESS },
        { GpuResourceFormat::BC5Unorm,          DL::TEX_FORMAT_BC5_UNORM,                  DL::TEX_FORMAT_BC5_TYPELESS },
        { GpuResourceFormat::BC5Snorm,          DL::TEX_FORMAT_BC5_SNORM,                  DL::TEX_FORMAT_BC5_TYPELESS },
        { GpuResourceFormat::BC6HU16,           DL::TEX_FORMAT_BC6H_UF16,                  DL::TEX_FORMAT_BC6H_TYPELESS },
        { GpuResourceFormat::BC6HS16,           DL::TEX_FORMAT_BC6H_SF16,                  DL::TEX_FORMAT_BC6H_TYPELESS },
        { GpuResourceFormat::BC7Unorm,          DL::TEX_FORMAT_BC7_UNORM,                  DL::TEX_FORMAT_BC7_TYPELESS },
        { GpuResourceFormat::BC7UnormSrgb,      DL::TEX_FORMAT_BC7_UNORM_SRGB,             DL::TEX_FORMAT_BC7_TYPELESS },

        { GpuResourceFormat::RGB16Float,        DL::TEX_FORMAT_UNKNOWN,                    DL::TEX_FORMAT_UNKNOWN },
        { GpuResourceFormat::RGB16Unorm,        DL::TEX_FORMAT_UNKNOWN,                    DL::TEX_FORMAT_UNKNOWN },
        { GpuResourceFormat::RGB16Uint,         DL::TEX_FORMAT_UNKNOWN,                    DL::TEX_FORMAT_UNKNOWN },
        { GpuResourceFormat::RGB16Snorm,        DL::TEX_FORMAT_UNKNOWN,                    DL::TEX_FORMAT_UNKNOWN },
        { GpuResourceFormat::RGB16Sint,         DL::TEX_FORMAT_UNKNOWN,                    DL::TEX_FORMAT_UNKNOWN },

        { GpuResourceFormat::RGB5A1Unorm,       DL::TEX_FORMAT_B5G5R5A1_UNORM,             DL::TEX_FORMAT_UNKNOWN },
        { GpuResourceFormat::RGB9E5Float,       DL::TEX_FORMAT_RGB9E5_SHAREDEXP,           DL::TEX_FORMAT_UNKNOWN },

        { GpuResourceFormat::BGRA8Unorm,        DL::TEX_FORMAT_BGRA8_UNORM,                DL::TEX_FORMAT_BGRA8_TYPELESS },
        { GpuResourceFormat::BGRA8UnormSrgb,    DL::TEX_FORMAT_BGRA8_UNORM_SRGB,           DL::TEX_FORMAT_BGRA8_TYPELESS },
        { GpuResourceFormat::BGRX8Unorm,        DL::TEX_FORMAT_BGRX8_UNORM,                DL::TEX_FORMAT_BGRA8_TYPELESS },
        { GpuResourceFormat::BGRX8UnormSrgb,    DL::TEX_FORMAT_BGRX8_UNORM_SRGB,           DL::TEX_FORMAT_BGRA8_TYPELESS },

        { GpuResourceFormat::R5G6B5Unorm,       DL::TEX_FORMAT_B5G6R5_UNORM,               DL::TEX_FORMAT_UNKNOWN },
    }; // clang-format on

    static_assert(eastl::is_same<std::underlying_type<GpuResourceFormat>::type, uint32_t>::value);
    static_assert(eastl::size(formatsConversion) == static_cast<uint32_t>(GpuResourceFormat::Count));

    DL::TEXTURE_FORMAT GetDLTextureFormat(GpuResourceFormat format)
    {
        ASSERT(format >= GpuResourceFormat::Unknown && format < GpuResourceFormat::Count);
        ASSERT(formatsConversion[static_cast<uint32_t>(format)].from == format);
        ASSERT(format == GpuResourceFormat::Unknown ||
               formatsConversion[static_cast<uint32_t>(format)].to != DL::TEX_FORMAT_UNKNOWN);

        return formatsConversion[static_cast<uint32_t>(format)].to;
    }
}