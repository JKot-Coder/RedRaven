#include "ImageWriter.hpp"

#include "gapi/MemoryAllocation.hpp"
#include "gapi/Texture.hpp"

#include "common/OnScopeExit.hpp"

#include "DirectXTex.h"

namespace OpenDemo
{
    namespace Tests
    {
        namespace
        {
            // Copy from DXGIFormatsUtils.hpp
            struct GpuResourceFormatConversion
            {
                GAPI::GpuResourceFormat from;
                ::DXGI_FORMAT to;
            };

            // clang-format off
                static GpuResourceFormatConversion formatsConversion[] = {
                    { GAPI::GpuResourceFormat::Unknown,           DXGI_FORMAT_UNKNOWN },
                    { GAPI::GpuResourceFormat::RGBA32Float,       DXGI_FORMAT_R32G32B32A32_FLOAT },
                    { GAPI::GpuResourceFormat::RGBA32Uint,        DXGI_FORMAT_R32G32B32A32_UINT },
                    { GAPI::GpuResourceFormat::RGBA32Sint,        DXGI_FORMAT_R32G32B32A32_SINT },
                    { GAPI::GpuResourceFormat::RGB32Float,        DXGI_FORMAT_R32G32B32_FLOAT },
                    { GAPI::GpuResourceFormat::RGB32Uint,         DXGI_FORMAT_R32G32B32_UINT },
                    { GAPI::GpuResourceFormat::RGB32Sint,         DXGI_FORMAT_R32G32B32_SINT },
                    { GAPI::GpuResourceFormat::RGBA16Float,       DXGI_FORMAT_R16G16B16A16_FLOAT },
                    { GAPI::GpuResourceFormat::RGBA16Unorm,       DXGI_FORMAT_R16G16B16A16_UNORM },
                    { GAPI::GpuResourceFormat::RGBA16Uint,        DXGI_FORMAT_R16G16B16A16_UINT },
                    { GAPI::GpuResourceFormat::RGBA16Snorm,       DXGI_FORMAT_R16G16B16A16_SNORM },
                    { GAPI::GpuResourceFormat::RGBA16Sint,        DXGI_FORMAT_R16G16B16A16_SINT },
                    { GAPI::GpuResourceFormat::RG32Float,         DXGI_FORMAT_R32G32_FLOAT },
                    { GAPI::GpuResourceFormat::RG32Uint,          DXGI_FORMAT_R32G32_UINT },
                    { GAPI::GpuResourceFormat::RG32Sint,          DXGI_FORMAT_R32G32_SINT },

                    { GAPI::GpuResourceFormat::RGB10A2Unorm,      DXGI_FORMAT_R10G10B10A2_UNORM },
                    { GAPI::GpuResourceFormat::RGB10A2Uint,       DXGI_FORMAT_R10G10B10A2_UINT },
                    { GAPI::GpuResourceFormat::R11G11B10Float,    DXGI_FORMAT_R11G11B10_FLOAT },
                    { GAPI::GpuResourceFormat::RGBA8Unorm,        DXGI_FORMAT_R8G8B8A8_UNORM },
                    { GAPI::GpuResourceFormat::RGBA8UnormSrgb,    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB },
                    { GAPI::GpuResourceFormat::RGBA8Uint,         DXGI_FORMAT_R8G8B8A8_UINT },
                    { GAPI::GpuResourceFormat::RGBA8Snorm,        DXGI_FORMAT_R8G8B8A8_SNORM },
                    { GAPI::GpuResourceFormat::RGBA8Sint,         DXGI_FORMAT_R8G8B8A8_SINT },
                    { GAPI::GpuResourceFormat::RG16Float,         DXGI_FORMAT_R16G16_FLOAT },
                    { GAPI::GpuResourceFormat::RG16Unorm,         DXGI_FORMAT_R16G16_UNORM },
                    { GAPI::GpuResourceFormat::RG16Uint,          DXGI_FORMAT_R16G16_UINT },
                    { GAPI::GpuResourceFormat::RG16Snorm,         DXGI_FORMAT_R16G16_SNORM },
                    { GAPI::GpuResourceFormat::RG16Sint,          DXGI_FORMAT_R16G16_SINT },

                    { GAPI::GpuResourceFormat::R32Float,          DXGI_FORMAT_R32_FLOAT },
                    { GAPI::GpuResourceFormat::R32Uint,           DXGI_FORMAT_R32_UINT },
                    { GAPI::GpuResourceFormat::R32Sint,           DXGI_FORMAT_R32_SINT },

                    { GAPI::GpuResourceFormat::RG8Unorm,          DXGI_FORMAT_R8G8_UNORM },
                    { GAPI::GpuResourceFormat::RG8Uint,           DXGI_FORMAT_R8G8_UINT },
                    { GAPI::GpuResourceFormat::RG8Snorm,          DXGI_FORMAT_R8G8_SNORM },
                    { GAPI::GpuResourceFormat::RG8Sint,           DXGI_FORMAT_R8G8_SINT },

                    { GAPI::GpuResourceFormat::R16Float,          DXGI_FORMAT_R16_FLOAT },
                    { GAPI::GpuResourceFormat::R16Unorm,          DXGI_FORMAT_R16_UNORM },
                    { GAPI::GpuResourceFormat::R16Uint,           DXGI_FORMAT_R16_UINT },
                    { GAPI::GpuResourceFormat::R16Snorm,          DXGI_FORMAT_R16_SNORM },
                    { GAPI::GpuResourceFormat::R16Sint,           DXGI_FORMAT_R16_SINT },
                    { GAPI::GpuResourceFormat::R8Unorm,           DXGI_FORMAT_R8_UNORM },
                    { GAPI::GpuResourceFormat::R8Uint,            DXGI_FORMAT_R8_UINT },
                    { GAPI::GpuResourceFormat::R8Snorm,           DXGI_FORMAT_R8_SNORM },
                    { GAPI::GpuResourceFormat::R8Sint,            DXGI_FORMAT_R8_SINT },
                    { GAPI::GpuResourceFormat::A8Unorm,           DXGI_FORMAT_A8_UNORM },

                    { GAPI::GpuResourceFormat::D32FloatS8X24Uint, DXGI_FORMAT_D32_FLOAT_S8X24_UINT },
                    { GAPI::GpuResourceFormat::D32Float,          DXGI_FORMAT_D32_FLOAT },
                    { GAPI::GpuResourceFormat::D24UnormS8Uint,    DXGI_FORMAT_D24_UNORM_S8_UINT },
                    { GAPI::GpuResourceFormat::D16Unorm,          DXGI_FORMAT_D16_UNORM },

                    { GAPI::GpuResourceFormat::R32FloatX8X24,     DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS },
                    { GAPI::GpuResourceFormat::X32G8Uint,         DXGI_FORMAT_X32_TYPELESS_G8X24_UINT },
                    { GAPI::GpuResourceFormat::R24UnormX8,        DXGI_FORMAT_R24_UNORM_X8_TYPELESS },
                    { GAPI::GpuResourceFormat::X24G8Uint,         DXGI_FORMAT_X24_TYPELESS_G8_UINT },

                    { GAPI::GpuResourceFormat::BC1Unorm,          DXGI_FORMAT_BC1_UNORM },
                    { GAPI::GpuResourceFormat::BC1UnormSrgb,      DXGI_FORMAT_BC1_UNORM_SRGB },
                    { GAPI::GpuResourceFormat::BC2Unorm,          DXGI_FORMAT_BC2_UNORM },
                    { GAPI::GpuResourceFormat::BC2UnormSrgb,      DXGI_FORMAT_BC2_UNORM_SRGB },
                    { GAPI::GpuResourceFormat::BC3Unorm,          DXGI_FORMAT_BC3_UNORM },
                    { GAPI::GpuResourceFormat::BC3UnormSrgb,      DXGI_FORMAT_BC3_UNORM_SRGB },
                    { GAPI::GpuResourceFormat::BC4Unorm,          DXGI_FORMAT_BC4_UNORM },
                    { GAPI::GpuResourceFormat::BC4Snorm,          DXGI_FORMAT_BC4_SNORM },
                    { GAPI::GpuResourceFormat::BC5Unorm,          DXGI_FORMAT_BC5_UNORM },
                    { GAPI::GpuResourceFormat::BC5Snorm,          DXGI_FORMAT_BC5_SNORM },
                    { GAPI::GpuResourceFormat::BC6HU16,           DXGI_FORMAT_BC6H_UF16 },
                    { GAPI::GpuResourceFormat::BC6HS16,           DXGI_FORMAT_BC6H_SF16 },
                    { GAPI::GpuResourceFormat::BC7Unorm,          DXGI_FORMAT_BC7_UNORM },
                    { GAPI::GpuResourceFormat::BC7UnormSrgb,      DXGI_FORMAT_BC7_UNORM_SRGB },

                    { GAPI::GpuResourceFormat::RGB16Float,        DXGI_FORMAT_UNKNOWN },
                    { GAPI::GpuResourceFormat::RGB16Unorm,        DXGI_FORMAT_UNKNOWN },
                    { GAPI::GpuResourceFormat::RGB16Uint,         DXGI_FORMAT_UNKNOWN },
                    { GAPI::GpuResourceFormat::RGB16Snorm,        DXGI_FORMAT_UNKNOWN },
                    { GAPI::GpuResourceFormat::RGB16Sint,         DXGI_FORMAT_UNKNOWN },

                    { GAPI::GpuResourceFormat::RGB5A1Unorm,       DXGI_FORMAT_B5G5R5A1_UNORM },
                    { GAPI::GpuResourceFormat::RGB9E5Float,       DXGI_FORMAT_R9G9B9E5_SHAREDEXP },

                    { GAPI::GpuResourceFormat::BGRA8Unorm,        DXGI_FORMAT_B8G8R8A8_UNORM },
                    { GAPI::GpuResourceFormat::BGRA8UnormSrgb,    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB },
                    { GAPI::GpuResourceFormat::BGRX8Unorm,        DXGI_FORMAT_B8G8R8X8_UNORM },
                    { GAPI::GpuResourceFormat::BGRX8UnormSrgb,    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB },

                    { GAPI::GpuResourceFormat::R5G6B5Unorm,       DXGI_FORMAT_B5G6R5_UNORM },
                }; // clang-format on

            static_assert(std::is_same<std::underlying_type<GAPI::GpuResourceFormat>::type, uint32_t>::value);
            static_assert(std::size(formatsConversion) == static_cast<uint32_t>(GAPI::GpuResourceFormat::Count));

            ::DXGI_FORMAT getDxgiResourceFormat(GAPI::GpuResourceFormat format)
            {
                ASSERT(formatsConversion[static_cast<uint32_t>(format)].from == format);
                ASSERT(format == GAPI::GpuResourceFormat::Unknown ||
                       formatsConversion[static_cast<uint32_t>(format)].to != DXGI_FORMAT_UNKNOWN);

                return formatsConversion[static_cast<uint32_t>(format)].to;
            }

            DirectX::TEX_DIMENSION getTextureDimension(GAPI::GpuResourceDimension dimension)
            {
                switch (dimension)
                {
                case GAPI::GpuResourceDimension::Texture1D:
                    return DirectX::TEX_DIMENSION_TEXTURE1D;
                case GAPI::GpuResourceDimension::Texture2D:
                case GAPI::GpuResourceDimension::Texture2DMS:
                case GAPI::GpuResourceDimension::TextureCube:
                    return DirectX::TEX_DIMENSION_TEXTURE2D;
                case GAPI::GpuResourceDimension::Texture3D:
                    return DirectX::TEX_DIMENSION_TEXTURE3D;
                default:
                    ASSERT_MSG(false, "Unsuported texture dimension");
                }
                return static_cast<DirectX::TEX_DIMENSION>(0);
            }

            DirectX::TexMetadata getTextureMetadata(const GAPI::CpuResourceData::SharedPtr& resource)
            {
                ASSERT(resource);
                ASSERT(resource->GetNumSubresources() > 0);

                const auto& resourceDesc = resource->GetResourceDescription();
                const auto firstSubresource = resource->GetSubresourceFootprintAt(0);

                ASSERT(resourceDesc.GetDimension() != GAPI::GpuResourceDimension::TextureCube);

                DirectX::TexMetadata metadata = {};
                metadata.width = resourceDesc.GetWidth();
                metadata.height = resourceDesc.GetHeight();
                metadata.depth = resourceDesc.GetDepth();
                metadata.arraySize = 1;
                metadata.mipLevels = resourceDesc.GetMipCount();
                metadata.format = getDxgiResourceFormat(resourceDesc.GetFormat());
                metadata.dimension = getTextureDimension(resourceDesc.GetDimension());

                return metadata;
            }

            void setupImageArray(std::vector<DirectX::Image>& images, const GAPI::CpuResourceData::SharedPtr& resource)
            {
                ASSERT(resource);

                const auto& resourceDesc = resource->GetResourceDescription();

                ASSERT(resourceDesc.GetNumSubresources() == resource->GetNumSubresources());
                ASSERT(resourceDesc.GetDimension() != GAPI::GpuResourceDimension::TextureCube);

                const auto dataPointer = static_cast<uint8_t*>(resource->GetAllocation()->Map());

                ON_SCOPE_EXIT(
                    {
                        resource->GetAllocation()->Unmap();
                    });

                for (uint32_t subresourceIdx = 0; subresourceIdx < resourceDesc.GetNumSubresources(); subresourceIdx++)
                {
                    const auto& subresourceFootprint = resource->GetSubresourceFootprintAt(subresourceIdx);
                    const auto arraySlice = resourceDesc.GetSubresourceArraySlice(subresourceIdx);
                    const auto mipLevel = resourceDesc.GetSubresourceMipLevel(subresourceIdx);

                    for (uint32_t slice = 0; slice < subresourceFootprint.depth; slice++)
                    {
                        DirectX::Image image;
                        image.width = subresourceFootprint.width;
                        image.height = subresourceFootprint.height; // Might be use rows??
                        image.format = getDxgiResourceFormat(resourceDesc.GetFormat());
                        image.rowPitch = subresourceFootprint.rowPitch;
                        image.slicePitch = subresourceFootprint.depthPitch;
                        image.pixels = dataPointer + subresourceFootprint.offset + subresourceFootprint.depthPitch * slice;
                        images.push_back(image);
                    }
                }
            }

            HRESULT saveToDDSFile(const GAPI::CpuResourceData::SharedPtr& resource, std::string path)
            {
                ASSERT(resource);

                const auto& metadata = getTextureMetadata(resource);

                const auto images = std::make_unique<std::vector<DirectX::Image>>();
                setupImageArray(*images, resource);

                return DirectX::SaveToDDSFile(images->data(), images->size(), metadata, DirectX::DDS_FLAGS_NONE, StringConversions::UTF8ToWString(path).c_str());
            }
            /*
            KTX_error_code setImageFromData(ktxTexture2* texture, const GAPI::CpuResourceData::SharedPtr& resource)
            {

                for (uint32_t subresourceIdx = 0; subresourceIdx < resourceDesc.GetNumSubresources(); subresourceIdx++)
                {
                    const auto& subresourceFootprint = resource->GetSubresourceFootprintAt(subresourceIdx);
                    const auto arraySlice = resourceDesc.GetSubresourceArraySlice(subresourceIdx);
                    const auto mipLevel = resourceDesc.GetSubresourceMipLevel(subresourceIdx);

                    ktx_size_t sliceDataSize = 0;
                    uint8_t* subresourceDataPointer = 0;
                    bool zeroPadding = subresourceFootprint.rowPitch == subresourceFootprint.rowSizeInBytes;

                    if (zeroPadding)
                    {
                        sliceDataSize = subresourceFootprint.depthPitch;
                        subresourceDataPointer = dataPointer + subresourceFootprint.offset;
                    }
                    else
                    {
                        sliceDataSize = subresourceFootprint.rowSizeInBytes * subresourceFootprint.numRows;
                        subresourceDataPointer = new uint8_t[sliceDataSize * subresourceFootprint.depth];

                        for (uint32_t row = 0; row < subresourceFootprint.numRows; row++)
                        {
                            const auto dst = subresourceDataPointer + row * subresourceFootprint.rowSizeInBytes;
                            const auto source = dataPointer + subresourceFootprint.offset + row * subresourceFootprint.rowPitch;
                            memcpy(dst, source, subresourceFootprint.rowSizeInBytes);
                        }
                    }

                    for (uint32_t faceSlice = 0; faceSlice < subresourceFootprint.depth; faceSlice++)
                    {
                        const auto slicePointer = subresourceDataPointer +
                                                  sliceDataSize * faceSlice;

                        const auto result = ktxTexture_SetImageFromMemory(ktxTexture(texture),
                                                                          mipLevel, arraySlice, faceSlice,
                                                                          slicePointer, sliceDataSize);
                        if (result != KTX_SUCCESS)
                            return result;
                    }

                    if (!zeroPadding)
                        delete[] subresourceDataPointer;
                }

                return KTX_SUCCESS;
            }*/
        }

        void ImageWriter::write(std::string path) const
        {
            ASSERT(resource_);

            const auto hr = saveToDDSFile(resource_, path);
            ASSERT(SUCCEEDED(hr));
        }
    }
}