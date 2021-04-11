#include "ImageWriter.hpp"

#include "gapi/Texture.hpp"

#include <../lib/vkformat_enum.h>
#include <ktx.h>

namespace OpenDemo
{
    namespace Tests
    {
        namespace
        {
            struct GpuResourceFormatConversion
            {
                GAPI::GpuResourceFormat from;
                VkFormat to;
            };

            // clang-format off
            static GpuResourceFormatConversion formatsConversion[] = {
                { GAPI::GpuResourceFormat::Unknown,           VK_FORMAT_UNDEFINED },                                                                
                { GAPI::GpuResourceFormat::RGBA32Float,       VK_FORMAT_R32G32B32A32_SFLOAT },
                { GAPI::GpuResourceFormat::RGBA32Uint,        VK_FORMAT_R32G32B32A32_UINT },
                { GAPI::GpuResourceFormat::RGBA32Sint,        VK_FORMAT_R32G32B32A32_SINT },
                { GAPI::GpuResourceFormat::RGB32Float,        VK_FORMAT_R32G32B32_SFLOAT },
                { GAPI::GpuResourceFormat::RGB32Uint,         VK_FORMAT_R32G32B32_UINT },
                { GAPI::GpuResourceFormat::RGB32Sint,         VK_FORMAT_R32G32B32_SINT },
                { GAPI::GpuResourceFormat::RGBA16Float,       VK_FORMAT_R16G16B16A16_SFLOAT },
                { GAPI::GpuResourceFormat::RGBA16Unorm,       VK_FORMAT_R16G16B16A16_UNORM },
                { GAPI::GpuResourceFormat::RGBA16Uint,        VK_FORMAT_R16G16B16A16_UINT },
                { GAPI::GpuResourceFormat::RGBA16Snorm,       VK_FORMAT_R16G16B16A16_SNORM },
                { GAPI::GpuResourceFormat::RGBA16Sint,        VK_FORMAT_R16G16B16A16_SINT },
                { GAPI::GpuResourceFormat::RG32Float,         VK_FORMAT_R32G32_SFLOAT },
                { GAPI::GpuResourceFormat::RG32Uint,          VK_FORMAT_R32G32_UINT },
                { GAPI::GpuResourceFormat::RG32Sint,          VK_FORMAT_R32G32_SINT },

                { GAPI::GpuResourceFormat::RGB10A2Unorm,      VK_FORMAT_A2R10G10B10_UNORM_PACK32 }, // Different component order?
                { GAPI::GpuResourceFormat::RGB10A2Uint,       VK_FORMAT_A2R10G10B10_UINT_PACK32 },  // Different component order?
                { GAPI::GpuResourceFormat::R11G11B10Float,    VK_FORMAT_B10G11R11_UFLOAT_PACK32 }, // Unsigned in VK
                { GAPI::GpuResourceFormat::RGBA8Unorm,        VK_FORMAT_R8G8B8A8_UNORM },
                { GAPI::GpuResourceFormat::RGBA8UnormSrgb,    VK_FORMAT_R8G8B8A8_SRGB },
                { GAPI::GpuResourceFormat::RGBA8Uint,         VK_FORMAT_R8G8B8A8_UINT },
                { GAPI::GpuResourceFormat::RGBA8Snorm,        VK_FORMAT_R8G8B8A8_SNORM },
                { GAPI::GpuResourceFormat::RGBA8Sint,         VK_FORMAT_R8G8B8A8_SINT },
                { GAPI::GpuResourceFormat::RG16Float,         VK_FORMAT_R16G16_SFLOAT },
                { GAPI::GpuResourceFormat::RG16Unorm,         VK_FORMAT_R16G16_UNORM },
                { GAPI::GpuResourceFormat::RG16Uint,          VK_FORMAT_R16G16_UINT },
                { GAPI::GpuResourceFormat::RG16Snorm,         VK_FORMAT_R16G16_SNORM },
                { GAPI::GpuResourceFormat::RG16Sint,          VK_FORMAT_R16G16_SINT },

                { GAPI::GpuResourceFormat::R32Float,          VK_FORMAT_R32_SFLOAT },
                { GAPI::GpuResourceFormat::R32Uint,           VK_FORMAT_R32_UINT },
                { GAPI::GpuResourceFormat::R32Sint,           VK_FORMAT_R32_SINT },

                { GAPI::GpuResourceFormat::RG8Unorm,          VK_FORMAT_R8G8_UNORM },
                { GAPI::GpuResourceFormat::RG8Uint,           VK_FORMAT_R8G8_UINT },
                { GAPI::GpuResourceFormat::RG8Snorm,          VK_FORMAT_R8G8_SNORM },
                { GAPI::GpuResourceFormat::RG8Sint,           VK_FORMAT_R8G8_SINT },

                { GAPI::GpuResourceFormat::R16Float,          VK_FORMAT_R16_SFLOAT },
                { GAPI::GpuResourceFormat::R16Unorm,          VK_FORMAT_R16_UNORM },
                { GAPI::GpuResourceFormat::R16Uint,           VK_FORMAT_R16_UINT },
                { GAPI::GpuResourceFormat::R16Snorm,          VK_FORMAT_R16_SNORM },
                { GAPI::GpuResourceFormat::R16Sint,           VK_FORMAT_R16_SINT },
                { GAPI::GpuResourceFormat::R8Unorm,           VK_FORMAT_R8_UNORM },
                { GAPI::GpuResourceFormat::R8Uint,            VK_FORMAT_R8_UINT },
                { GAPI::GpuResourceFormat::R8Snorm,           VK_FORMAT_R8_SNORM },  
                { GAPI::GpuResourceFormat::R8Sint,            VK_FORMAT_R8_SINT },
                { GAPI::GpuResourceFormat::A8Unorm,           VK_FORMAT_R8_UNORM },

                { GAPI::GpuResourceFormat::D32FloatS8X24Uint, VK_FORMAT_D32_SFLOAT_S8_UINT }, // Warning
                { GAPI::GpuResourceFormat::D32Float,          VK_FORMAT_D32_SFLOAT },
                { GAPI::GpuResourceFormat::D24UnormS8Uint,    VK_FORMAT_D24_UNORM_S8_UINT },
                { GAPI::GpuResourceFormat::D16Unorm,          VK_FORMAT_D16_UNORM },

                { GAPI::GpuResourceFormat::R32FloatX8X24,     VK_FORMAT_UNDEFINED },
                { GAPI::GpuResourceFormat::X32G8Uint,         VK_FORMAT_UNDEFINED },
                { GAPI::GpuResourceFormat::R24UnormX8,        VK_FORMAT_UNDEFINED },
                { GAPI::GpuResourceFormat::X24G8Uint,         VK_FORMAT_UNDEFINED },

                { GAPI::GpuResourceFormat::BC1Unorm,          VK_FORMAT_BC1_RGBA_UNORM_BLOCK },
                { GAPI::GpuResourceFormat::BC1UnormSrgb,      VK_FORMAT_BC1_RGBA_SRGB_BLOCK },
                { GAPI::GpuResourceFormat::BC2Unorm,          VK_FORMAT_BC2_UNORM_BLOCK },
                { GAPI::GpuResourceFormat::BC2UnormSrgb,      VK_FORMAT_BC2_SRGB_BLOCK },
                { GAPI::GpuResourceFormat::BC3Unorm,          VK_FORMAT_BC3_UNORM_BLOCK },
                { GAPI::GpuResourceFormat::BC3UnormSrgb,      VK_FORMAT_BC3_SRGB_BLOCK },
                { GAPI::GpuResourceFormat::BC4Unorm,          VK_FORMAT_BC4_UNORM_BLOCK },
                { GAPI::GpuResourceFormat::BC4Snorm,          VK_FORMAT_BC4_SNORM_BLOCK },
                { GAPI::GpuResourceFormat::BC5Unorm,          VK_FORMAT_BC5_UNORM_BLOCK },
                { GAPI::GpuResourceFormat::BC5Snorm,          VK_FORMAT_BC5_SNORM_BLOCK },
                { GAPI::GpuResourceFormat::BC6HU16,           VK_FORMAT_BC6H_UFLOAT_BLOCK },
                { GAPI::GpuResourceFormat::BC6HS16,           VK_FORMAT_BC6H_SFLOAT_BLOCK },
                { GAPI::GpuResourceFormat::BC7Unorm,          VK_FORMAT_BC7_UNORM_BLOCK },
                { GAPI::GpuResourceFormat::BC7UnormSrgb,      VK_FORMAT_BC7_SRGB_BLOCK },

                { GAPI::GpuResourceFormat::RGB16Float,        VK_FORMAT_UNDEFINED },
                { GAPI::GpuResourceFormat::RGB16Unorm,        VK_FORMAT_UNDEFINED },
                { GAPI::GpuResourceFormat::RGB16Uint,         VK_FORMAT_UNDEFINED },
                { GAPI::GpuResourceFormat::RGB16Snorm,        VK_FORMAT_UNDEFINED },
                { GAPI::GpuResourceFormat::RGB16Sint,         VK_FORMAT_UNDEFINED },

                { GAPI::GpuResourceFormat::RGB5A1Unorm,       VK_FORMAT_B5G5R5A1_UNORM_PACK16 }, // Different component order?
                { GAPI::GpuResourceFormat::RGB9E5Float,       VK_FORMAT_E5B9G9R9_UFLOAT_PACK32 }, // Unsigned in VK

                { GAPI::GpuResourceFormat::BGRA8Unorm,        VK_FORMAT_B8G8R8A8_UNORM },
                { GAPI::GpuResourceFormat::BGRA8UnormSrgb,    VK_FORMAT_B8G8R8A8_SRGB },
                { GAPI::GpuResourceFormat::BGRX8Unorm,        VK_FORMAT_B8G8R8A8_UNORM },
                { GAPI::GpuResourceFormat::BGRX8UnormSrgb,    VK_FORMAT_B8G8R8A8_SRGB },

                { GAPI::GpuResourceFormat::R5G6B5Unorm,       VK_FORMAT_R5G6B5_UNORM_PACK16 },
            }; // clang-format on

            static_assert(std::is_same<std::underlying_type<GAPI::GpuResourceFormat>::type, uint32_t>::value);
            static_assert(std::size(formatsConversion) == static_cast<uint32_t>(GAPI::GpuResourceFormat::Count));

            VkFormat getVkResourceFormat(GAPI::GpuResourceFormat format)
            {
                ASSERT(formatsConversion[static_cast<uint32_t>(format)].from == format);
                ASSERT(format == GAPI::GpuResourceFormat::Unknown ||
                       formatsConversion[static_cast<uint32_t>(format)].to != VK_FORMAT_UNDEFINED);

                return formatsConversion[static_cast<uint32_t>(format)].to;
            }

            const ktxTextureCreateInfo& getTextureCreateInfo(const GAPI::CpuResourceData::SharedPtr& resource)
            {
                ASSERT(resource);
                ASSERT(resource->GetNumSubresources() > 0);

                ktxTextureCreateInfo createInfo;

                const auto firstSubresource = resource->GetSubresourceFootprintAt(0);

                createInfo.vkFormat = GetVkResourceFormat(resource->GetFormat());
                createInfo.baseWidth = firstSubresource.width;
                createInfo.baseHeight = firstSubresource.height;
                createInfo.baseDepth = firstSubresource.depth;
                createInfo.numDimensions = resource.;
                // Note: it is not necessary to provide a full mipmap pyramid.
                createInfo.numLevels = log2(createInfo.baseWidth) + 1;
                createInfo.numLayers = 1;
                createInfo.numFaces = 1;
                createInfo.isArray = KTX_FALSE;
                createInfo.generateMipmaps = KTX_FALSE;

                return createInfo;
            }
        }

        void ImageWriter::write(std::string path) const
        {
            ASSERT(resource_);

            ktxTexture2* texture;

            KTX_error_code result;
            ktx_uint32_t level, layer, faceSlice;
            uint8_t* src;
            ktx_size_t srcSize;

            result = ktxTexture2_Create(&createInfo,
                                        KTX_TEXTURE_CREATE_ALLOC_STORAGE,
                                        &texture);

            /*  src = // Open a stdio FILE* on the baseLevel image, slice 0.
                srcSize = // Query size of the file.
                level = 0;
            layer = 0;
            faceSlice = 0;*/
            //  result = ktxTexture_SetImageFromMemory(ktxTexture(texture),
            //                                         level, layer, faceSlice,
            //                                        src, srcSize);

            // Repeat for the other 15 slices of the base level and all other levels
            // up to createInfo.numLevels.
            ktxTexture_WriteToNamedFile(ktxTexture(texture), path.c_str());
            ktxTexture_Destroy(ktxTexture(texture));
        }
    }
}