#include "Resource.hpp"

#include "gapi/Buffer.hpp"
#include "gapi/Texture.hpp"

#include "common/Math.hpp"

namespace RR
{
    namespace GAPI
    {
        namespace
        {
            enum class FormatType
            {
                Unknown, ///< Unknown format Type
                Float, ///< Floating-point formats
                Unorm, ///< Unsigned normalized formats
                UnormSrgb, ///< Unsigned normalized SRGB formats
                Snorm, ///< Signed normalized formats
                Uint, ///< Unsigned integer formats
                Sint ///< Signed integer formats
            };

            struct FormatInfo
            {
                GpuResourceFormat format;
                U8String name;

                FormatType type;

                uint32_t blockSize;
                uint32_t channelCount;

                struct
                {
                    uint32_t width;
                    uint32_t height;
                } compressionBlock;

                uint32_t channelBits[4];

                bool isDepth;
                bool isStencil;
                bool isCompressed;
            };

            // clang-format off
            static FormatInfo formatInfo[] = {
                    //Format                              Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w] isDepth isStencil isCompressed
                    GpuResourceFormat::Unknown,           u8"Unknown",           FormatType::Unknown,   0,         0,                      {1, 1},            0,   0,  0,  0,  false,  false,    false,
                    GpuResourceFormat::RGBA32Float,       u8"RGBA32Float",       FormatType::Float,     16,        4,                      {1, 1},            32,  32, 32, 32, false,  false,    false,
                    GpuResourceFormat::RGBA32Uint,        u8"RGBA32Uint",        FormatType::Uint,      16,        4,                      {1, 1},            32,  32, 32, 32, false,  false,    false,
                    GpuResourceFormat::RGBA32Sint,        u8"RGBA32Sint",        FormatType::Sint,      16,        4,                      {1, 1},            32,  32, 32, 32, false,  false,    false,
                    GpuResourceFormat::RGB32Float,        u8"RGB32Float",        FormatType::Float,     12,        3,                      {1, 1},            32,  32, 32, 0,  false,  false,    false,
                    GpuResourceFormat::RGB32Uint,         u8"RGB32Uint",         FormatType::Uint,      12,        3,                      {1, 1},            32,  32, 32, 0,  false,  false,    false,
                    GpuResourceFormat::RGB32Sint,         u8"RGB32Sint",         FormatType::Sint,      12,        3,                      {1, 1},            32,  32, 32, 0,  false,  false,    false,
                    GpuResourceFormat::RGBA16Float,       u8"RGBA16Float",       FormatType::Float,     8,         4,                      {1, 1},            16,  16, 16, 16, false,  false,    false,
                    GpuResourceFormat::RGBA16Unorm,       u8"RGBA16Unorm",       FormatType::Unorm,     8,         4,                      {1, 1},            16,  16, 16, 16, false,  false,    false,
                    GpuResourceFormat::RGBA16Uint,        u8"RGBA16Uint",        FormatType::Uint,      8,         4,                      {1, 1},            16,  16, 16, 16, false,  false,    false,
                    GpuResourceFormat::RGBA16Snorm,       u8"RGBA16Snorm",       FormatType::Snorm,     8,         4,                      {1, 1},            16,  16, 16, 16, false,  false,    false,
                    GpuResourceFormat::RGBA16Sint,        u8"RGBA16Sint",        FormatType::Sint,      8,         4,                      {1, 1},            16,  16, 16, 16, false,  false,    false,
                    GpuResourceFormat::RG32Float,         u8"RG32Float",         FormatType::Float,     8,         2,                      {1, 1},            32,  32, 0,  0,  false,  false,    false,
                    GpuResourceFormat::RG32Uint,          u8"RG32Uint",          FormatType::Uint,      8,         2,                      {1, 1},            32,  32, 0,  0,  false,  false,    false,
                    GpuResourceFormat::RG32Sint,          u8"RG32Sint",          FormatType::Sint,      8,         2,                      {1, 1},            32,  32, 0,  0,  false,  false,    false,
                    //Format                              Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                    GpuResourceFormat::RGB10A2Unorm,      u8"RGB10A2Unorm",      FormatType::Unorm,     4,         4,                      {1, 1},            10,  10, 10, 2,  false,  false,    false,
                    GpuResourceFormat::RGB10A2Uint,       u8"RGB10A2Uint",       FormatType::Uint,      4,         4,                      {1, 1},            10,  10, 10, 2,  false,  false,    false,
                    GpuResourceFormat::R11G11B10Float,    u8"R11G11B10Float",    FormatType::Float,     4,         3,                      {1, 1},            11,  11, 10, 0,  false,  false,    false,
                    GpuResourceFormat::RGBA8Unorm,        u8"RGBA8Unorm",        FormatType::Unorm,     4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                    GpuResourceFormat::RGBA8UnormSrgb,    u8"RGBA8UnormSrgb",    FormatType::UnormSrgb, 4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                    GpuResourceFormat::RGBA8Uint,         u8"RGBA8Uint",         FormatType::Uint,      4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                    GpuResourceFormat::RGBA8Snorm,        u8"RGBA8Snorm",        FormatType::Snorm,     4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                    GpuResourceFormat::RGBA8Sint,         u8"RGBA8Sint",         FormatType::Sint,      4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                    GpuResourceFormat::RG16Float,         u8"RG16Float",         FormatType::Float,     4,         2,                      {1, 1},            16,  16, 0,  0,  false,  false,    false,
                    GpuResourceFormat::RG16Unorm,         u8"RG16Unorm",         FormatType::Unorm,     4,         2,                      {1, 1},            16,  16, 0,  0,  false,  false,    false,
                    GpuResourceFormat::RG16Uint,          u8"RG16Uint",          FormatType::Uint,      4,         2,                      {1, 1},            16,  16, 0,  0,  false,  false,    false,
                    GpuResourceFormat::RG16Snorm,         u8"RG16Snorm",         FormatType::Snorm,     4,         2,                      {1, 1},            16,  16, 0,  0,  false,  false,    false,
                    GpuResourceFormat::RG16Sint,          u8"RG16Sint",          FormatType::Sint,      4,         2,                      {1, 1},            16,  16, 0,  0,  false,  false,    false,
                    //Format                              Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                    GpuResourceFormat::R32Float,          u8"R32Float",          FormatType::Float,     4,         1,                      {1, 1},            32,  0,  0,  0,  false,  false,    false,
                    GpuResourceFormat::R32Uint,           u8"R32Uint",           FormatType::Uint,      4,         1,                      {1, 1},            32,  0,  0,  0,  false,  false,    false,
                    GpuResourceFormat::R32Sint,           u8"R32Sint",           FormatType::Sint,      4,         1,                      {1, 1},            32,  0,  0,  0,  false,  false,    false,
                    //Format                              Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                    GpuResourceFormat::RG8Unorm,          u8"RG8Unorm",          FormatType::Unorm,     2,         2,                      {1, 1},            8,   8,  0,  0,  false,  false,    false,
                    GpuResourceFormat::RG8Uint,           u8"RG8Uint",           FormatType::Uint,      2,         2,                      {1, 1},            8,   8,  0,  0,  false,  false,    false,
                    GpuResourceFormat::RG8Snorm,          u8"RG8Snorm",          FormatType::Snorm,     2,         2,                      {1, 1},            8,   8,  0,  0,  false,  false,    false,
                    GpuResourceFormat::RG8Sint,           u8"RG8Sint",           FormatType::Sint,      2,         2,                      {1, 1},            8,   8,  0,  0,  false,  false,    false,
                    //Format                              Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                    GpuResourceFormat::R16Float,          u8"R16Float",          FormatType::Float,     2,         1,                      {1, 1},            16,  0,  0,  0,  false,  false,    false,
                    GpuResourceFormat::R16Unorm,          u8"R16Unorm",          FormatType::Unorm,     2,         1,                      {1, 1},            16,  0,  0,  0,  false,  false,    false,
                    GpuResourceFormat::R16Uint,           u8"R16Uint",           FormatType::Uint,      2,         1,                      {1, 1},            16,  0,  0,  0,  false,  false,    false,
                    GpuResourceFormat::R16Snorm,          u8"R16Snorm",          FormatType::Snorm,     2,         1,                      {1, 1},            16,  0,  0,  0,  false,  false,    false,
                    GpuResourceFormat::R16Sint,           u8"R16Sint",           FormatType::Sint,      2,         1,                      {1, 1},            16,  0,  0,  0,  false,  false,    false,
                    GpuResourceFormat::R8Unorm,           u8"R8Unorm",           FormatType::Unorm,     1,         1,                      {1, 1},            8,   0,  0,  0,  false,  false,    false,
                    GpuResourceFormat::R8Uint,            u8"R8Uint",            FormatType::Uint,      1,         1,                      {1, 1},            8,   0,  0,  0,  false,  false,    false,
                    GpuResourceFormat::R8Snorm,           u8"R8Snorm",           FormatType::Snorm,     1,         1,                      {1, 1},            8,   0,  0,  0,  false,  false,    false,
                    GpuResourceFormat::R8Sint,            u8"R8Sint",            FormatType::Sint,      1,         1,                      {1, 1},            8,   0,  0,  0,  false,  false,    false,
                    GpuResourceFormat::A8Unorm,           u8"A8Unorm",           FormatType::Unorm,     1,         1,                      {1, 1},            0,   0,  0,  8,  false,  false,    false,
                    //Format                              Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                    GpuResourceFormat::D32FloatS8X24Uint, u8"D32FloatS8X24Uint", FormatType::Float,     8,         2,                      {1, 1},            32,  8,  24, 0,  true,   true,     false,
                    GpuResourceFormat::D32Float,          u8"D32Float",          FormatType::Float,     4,         1,                      {1, 1},            32,  0,  0,  0,  true,   false,    false,
                    GpuResourceFormat::D24UnormS8Uint,    u8"D24UnormS8Uint",    FormatType::Unorm,     4,         2,                      {1, 1},            24,  8,  0,  0,  true,   true,     false,
                    GpuResourceFormat::D16Unorm,          u8"D16Unorm",          FormatType::Unorm,     2,         1,                      {1, 1},            16,  0,  0,  0,  true,   false,    false,
                    //Format                              Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                    GpuResourceFormat::R32FloatX8X24,     u8"R32FloatX8X24",     FormatType::Float,     8,         2,                      {1, 1},            32,  8, 24,  0,  false,  false,    false,
                    GpuResourceFormat::X32G8Uint,         u8"X32G8Uint",         FormatType::Uint,      8,         2,                      {1, 1},            32,  8,  0,  0,  false,  false,    false,
                    GpuResourceFormat::R24UnormX8,        u8"R24UnormX8",        FormatType::Unorm,     4,         2,                      {1, 1},            24,  8,  0,  0,  false,  false,    false,
                    GpuResourceFormat::X24G8Uint,         u8"X24G8Uint",         FormatType::Uint,      4,         2,                      {1, 1},            24,  8,  0,  0,  false,  false,    false,
                    //Format                              Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                    GpuResourceFormat::BC1Unorm,          u8"BC1Unorm",          FormatType::Unorm,     8,         3,                      {4, 4},            64,  0,  0,  0,  false,  false,    true,
                    GpuResourceFormat::BC1UnormSrgb,      u8"BC1UnormSrgb",      FormatType::UnormSrgb, 8,         3,                      {4, 4},            64,  0,  0,  0,  false,  false,    true,
                    GpuResourceFormat::BC2Unorm,          u8"BC2Unorm",          FormatType::Unorm,     16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                    GpuResourceFormat::BC2UnormSrgb,      u8"BC2UnormSrgb",      FormatType::UnormSrgb, 16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                    GpuResourceFormat::BC3Unorm,          u8"BC3Unorm",          FormatType::Unorm,     16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                    GpuResourceFormat::BC3UnormSrgb,      u8"BC3UnormSrgb",      FormatType::UnormSrgb, 16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                    GpuResourceFormat::BC4Unorm,          u8"BC4Unorm",          FormatType::Unorm,     8,         1,                      {4, 4},            64,  0,  0,  0,  false,  false,    true,
                    GpuResourceFormat::BC4Snorm,          u8"BC4Snorm",          FormatType::Snorm,     8,         1,                      {4, 4},            64,  0,  0,  0,  false,  false,    true,
                    GpuResourceFormat::BC5Unorm,          u8"BC5Unorm",          FormatType::Unorm,     16,        2,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                    GpuResourceFormat::BC5Snorm,          u8"BC5Snorm",          FormatType::Snorm,     16,        2,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                    GpuResourceFormat::BC6HU16,           u8"BC6HU16",           FormatType::Float,     16,        3,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                    GpuResourceFormat::BC6HS16,           u8"BC6HS16",           FormatType::Float,     16,        3,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                    GpuResourceFormat::BC7Unorm,          u8"BC7Unorm",          FormatType::Unorm,     16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                    GpuResourceFormat::BC7UnormSrgb,      u8"BC7UnormSrgb",      FormatType::UnormSrgb, 16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                    //Format                              Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                    GpuResourceFormat::RGB16Float,        u8"RGB16Float",        FormatType::Float,     6,         3,                      {1, 1},            16,  16, 16, 0,  false,  false,    false,
                    GpuResourceFormat::RGB16Unorm,        u8"RGB16Unorm",        FormatType::Unorm,     6,         3,                      {1, 1},            16,  16, 16, 0,  false,  false,    false,
                    GpuResourceFormat::RGB16Uint,         u8"RGB16Uint",         FormatType::Uint,      6,         3,                      {1, 1},            16,  16, 16, 0,  false,  false,    false,
                    GpuResourceFormat::RGB16Snorm,        u8"RGB16Snorm",        FormatType::Snorm,     6,         3,                      {1, 1},            16,  16, 16, 0,  false,  false,    false,
                    GpuResourceFormat::RGB16Sint,         u8"RGB16Sint",         FormatType::Sint,      6,         3,                      {1, 1},            16,  16, 16, 0,  false,  false,    false,
                     //Format                             Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                    GpuResourceFormat::RGB5A1Unorm,       u8"RGB5A1Unorm",       FormatType::Unorm,     2,         4,                      {1, 1},            5,   5,  5,  1,  false,  false,    false,
                    GpuResourceFormat::RGB9E5Float,       u8"RGB9E5Float",       FormatType::Float,     4,         3,                      {1, 1},            9,   9,  9,  5,  false,  false,    false,
                    //Format                              Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                    GpuResourceFormat::BGRA8Unorm,        u8"BGRA8Unorm",        FormatType::Unorm,     4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                    GpuResourceFormat::BGRA8UnormSrgb,    u8"BGRA8UnormSrgb",    FormatType::UnormSrgb, 4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                    GpuResourceFormat::BGRX8Unorm,        u8"BGRX8Unorm",        FormatType::Unorm,     4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                    GpuResourceFormat::BGRX8UnormSrgb,    u8"BGRX8UnormSrgb",    FormatType::UnormSrgb, 4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                    //Format                              Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                    GpuResourceFormat::R5G6B5Unorm,       u8"R5G6B5Unorm",       FormatType::Unorm,     2,         3,                      {1, 1},            5,   6,  5,  0,  false,  false,    false,
            };
            // clang-format on

            static_assert(std::is_same<std::underlying_type<GpuResourceFormat>::type, uint32_t>::value);
            static_assert(std::size(formatInfo) == static_cast<uint32_t>(GpuResourceFormat::Count));
        }

        namespace GpuResourceFormatInfo
        {
            bool IsDepth(GpuResourceFormat format)
            {
                ASSERT(format == formatInfo[static_cast<uint32_t>(format)].format);
                return formatInfo[static_cast<uint32_t>(format)].isDepth;
            }

            bool IsStencil(GpuResourceFormat format)
            {
                ASSERT(format == formatInfo[static_cast<uint32_t>(format)].format);
                return formatInfo[static_cast<uint32_t>(format)].isStencil;
            }

            bool IsCompressed(GpuResourceFormat format)
            {
                ASSERT(format == formatInfo[static_cast<uint32_t>(format)].format);
                return formatInfo[static_cast<uint32_t>(format)].isCompressed;
            }

            uint32_t GetBlockSize(GpuResourceFormat format)
            {
                ASSERT(format == formatInfo[static_cast<uint32_t>(format)].format);
                return formatInfo[static_cast<uint32_t>(format)].blockSize;
            }

            uint32_t GetCompressionBlockWidth(GpuResourceFormat format)
            {
                ASSERT(format == formatInfo[static_cast<uint32_t>(format)].format);
                return formatInfo[static_cast<uint32_t>(format)].compressionBlock.width;
            }

            uint32_t GetCompressionBlockHeight(GpuResourceFormat format)
            {
                ASSERT(format == formatInfo[static_cast<uint32_t>(format)].format);
                return formatInfo[static_cast<uint32_t>(format)].compressionBlock.height;
            }

            U8String ToString(GpuResourceFormat format)
            {
                ASSERT(format == formatInfo[static_cast<uint32_t>(format)].format);
                return formatInfo[static_cast<uint32_t>(format)].name;
            }
        }

        bool GpuResourceDescription::IsValid() const
        {
            if (dimension == GpuResourceDimension::Buffer)
            {
                if (buffer.size < 1)
                {
                    LOG_WARNING("Wrong size of resource");
                    return false;
                }

                if ((IsSet(buffer.flags, BufferFlags::RawBuffer) +
                     IsSet(buffer.flags, BufferFlags::IndexBuffer) +
                     IsSet(buffer.flags, BufferFlags::StructuredBuffer)) > 1)
                {
                    LOG_WARNING("Buffer can't be a RawBuffer/IndexBuffer/StructuredBuffer at the same time");
                    return false;
                }

                if (IsSet(buffer.flags, BufferFlags::IndexBuffer) && (buffer.stride != 2 && buffer.stride != 4))
                {
                    LOG_WARNING("Index buffer stride must be 2 or 4");
                    return false;
                }

                if (IsSet(buffer.flags, BufferFlags::StructuredBuffer) && (buffer.stride == 0))
                {
                    LOG_WARNING("Structured buffer stride cannot be zero");
                    return false;
                }

                if (!IsAny(buffer.flags, BufferFlags::StructuredBuffer | BufferFlags::IndexBuffer) && (buffer.stride != 0))
                {
                    LOG_WARNING("Stride of non structured buffer should be zero");
                    return false;
                }

                if (IsSet(buffer.flags, BufferFlags::RawBuffer) && (buffer.size % 4 != 0))
                {
                    LOG_WARNING("The size of the raw buffer must be a multiple of 4");
                    return false;
                }

                if (IsSet(bindFlags, GpuResourceBindFlags::RenderTarget))
                {
                    LOG_WARNING("Buffer can't be binded as RenderTarget");
                    return false;
                }

                if ((buffer.stride != 0) && (buffer.size % buffer.stride != 0))
                {
                    LOG_WARNING("The size of the buffer must be a multiple of the stride");
                    return false;
                }
            }
            else
            {
                if ((texture.mipLevels > GetMaxMipLevel()) || (texture.mipLevels <= 0))
                {
                    LOG_WARNING("MipMaps levels must be greater zero and less or equal MaxMipLevel");
                    return false;
                }

                if (texture.format == GpuResourceFormat::Unknown)
                {
                    LOG_WARNING("Unknown resource format");
                    return false;
                }

                if (dimension == GpuResourceDimension::Texture2DMS && !IsSet(bindFlags, GpuResourceBindFlags::RenderTarget))
                {
                    LOG_WARNING("Multisampled texture must be allowed to bind as render target");
                    return false;
                }

                if (((texture.multisampleType != MultisampleType::None) && (dimension != GpuResourceDimension::Texture2DMS)) ||
                    ((texture.multisampleType == MultisampleType::None) && (dimension == GpuResourceDimension::Texture2DMS)))
                {
                    LOG_WARNING("Wrong multisample type");
                    return false;
                }

                if (texture.width < 1)
                {
                    LOG_WARNING("Wrong size of resource");
                    return false;
                }

                switch (dimension)
                {
                    case GpuResourceDimension::Texture1D:
                        if ((texture.depth != 1) || (texture.height != 1))
                        {
                            LOG_WARNING("Wrong texture size");
                            return false;
                        }
                        break;
                    case GpuResourceDimension::Texture2D:
                    case GpuResourceDimension::TextureCube:
                    case GpuResourceDimension::Texture2DMS:
                        if ((texture.height < 1) || (texture.depth != 1))
                        {
                            LOG_WARNING("Wrong texture size");
                            return false;
                        }
                        break;
                    case GpuResourceDimension::Texture3D:
                        if ((texture.height < 1) || (texture.depth < 1))
                        {
                            LOG_WARNING("Wrong texture size");
                            return false;
                        }

                        if (texture.arraySize != 1)
                        {
                            LOG_WARNING("Wrong size of array");
                            return false;
                        }
                        break;
                    default:
                        LOG_FATAL("Unsupported resource dimension");
                }

                if (GpuResourceFormatInfo::IsCompressed(texture.format))
                {
                    if ((dimension != GpuResourceDimension::Texture2D) && (dimension != GpuResourceDimension::TextureCube))
                    {
                        LOG_WARNING("Compressed resource formats only allowed for 2D and Cube textures");
                        return false;
                    }

                    if ((AlignTo(texture.width, GpuResourceFormatInfo::GetCompressionBlockWidth(texture.format)) != texture.width) ||
                        (AlignTo(texture.height, GpuResourceFormatInfo::GetCompressionBlockHeight(texture.format)) == texture.height))
                    {
                        LOG_WARNING("Size of compressed texture must be aligned to CompressionBlock");
                        return false;
                    }
                }
            }

            return true;
        }

        template <>
        std::shared_ptr<Texture> GpuResource::GetTyped<Texture>()
        {
            ASSERT(description_.IsTexture());
            //TODO inherit_shared_from_this
            return std::static_pointer_cast<Texture>(shared_from_this());
        }

        template <>
        std::shared_ptr<Buffer> GpuResource::GetTyped<Buffer>()
        {
            ASSERT(description_.IsBuffer());
            //TODO inherit_shared_from_this
            return std::static_pointer_cast<Buffer>(shared_from_this());
        }
    }
}