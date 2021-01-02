#include "Resource.hpp"

#include "gapi/Texture.hpp"
//#include "gapi/Buffer.hpp"

namespace OpenDemo
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
                ResourceFormat format;
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
                //Format                           Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w] isDepth isStencil isCompressed
                ResourceFormat::Unknown,           u8"Unknown",           FormatType::Unknown,   0,         0,                      {1, 1},            0,   0,  0,  0,  false,  false,    false,
                ResourceFormat::RGBA32Float,       u8"RGBA32Float",       FormatType::Float,     16,        4,                      {1, 1},            32,  32, 32, 32, false,  false,    false,
                ResourceFormat::RGBA32Uint,        u8"RGBA32Uint",        FormatType::Uint,      16,        4,                      {1, 1},            32,  32, 32, 32, false,  false,    false,
                ResourceFormat::RGBA32Sint,        u8"RGBA32Sint",        FormatType::Sint,      16,        4,                      {1, 1},            32,  32, 32, 32, false,  false,    false,
                ResourceFormat::RGB32Float,        u8"RGB32Float",        FormatType::Float,     12,        3,                      {1, 1},            32,  32, 32, 0,  false,  false,    false,
                ResourceFormat::RGB32Uint,         u8"RGB32Uint",         FormatType::Uint,      12,        3,                      {1, 1},            32,  32, 32, 0,  false,  false,    false,
                ResourceFormat::RGB32Sint,         u8"RGB32Sint",         FormatType::Sint,      12,        3,                      {1, 1},            32,  32, 32, 0,  false,  false,    false,
                ResourceFormat::RGBA16Float,       u8"RGBA16Float",       FormatType::Float,     8,         4,                      {1, 1},            16,  16, 16, 16, false,  false,    false,
                ResourceFormat::RGBA16Unorm,       u8"RGBA16Unorm",       FormatType::Unorm,     8,         4,                      {1, 1},            16,  16, 16, 16, false,  false,    false,
                ResourceFormat::RGBA16Uint,        u8"RGBA16Uint",        FormatType::Uint,      8,         4,                      {1, 1},            16,  16, 16, 16, false,  false,    false,
                ResourceFormat::RGBA16Snorm,       u8"RGBA16Snorm",       FormatType::Snorm,     8,         4,                      {1, 1},            16,  16, 16, 16, false,  false,    false,
                ResourceFormat::RGBA16Sint,        u8"RGBA16Sint",        FormatType::Sint,      8,         4,                      {1, 1},            16,  16, 16, 16, false,  false,    false,
                ResourceFormat::RG32Float,         u8"RG32Float",         FormatType::Float,     8,         2,                      {1, 1},            32,  32, 0,  0,  false,  false,    false,
                ResourceFormat::RG32Uint,          u8"RG32Uint",          FormatType::Uint,      8,         2,                      {1, 1},            32,  32, 0,  0,  false,  false,    false,
                ResourceFormat::RG32Sint,          u8"RG32Sint",          FormatType::Sint,      8,         2,                      {1, 1},            32,  32, 0,  0,  false,  false,    false,
                //Format                           Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                ResourceFormat::RGB10A2Unorm,      u8"RGB10A2Unorm",      FormatType::Unorm,     4,         4,                      {1, 1},            10,  10, 10, 2,  false,  false,    false,
                ResourceFormat::RGB10A2Uint,       u8"RGB10A2Uint",       FormatType::Uint,      4,         4,                      {1, 1},            10,  10, 10, 2,  false,  false,    false,
                ResourceFormat::R11G11B10Float,    u8"R11G11B10Float",    FormatType::Float,     4,         3,                      {1, 1},            11,  11, 10, 0,  false,  false,    false,
                ResourceFormat::RGBA8Unorm,        u8"RGBA8Unorm",        FormatType::Unorm,     4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                ResourceFormat::RGBA8UnormSrgb,    u8"RGBA8UnormSrgb",    FormatType::UnormSrgb, 4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                ResourceFormat::RGBA8Uint,         u8"RGBA8Uint",         FormatType::Uint,      4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                ResourceFormat::RGBA8Snorm,        u8"RGBA8Snorm",        FormatType::Snorm,     4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                ResourceFormat::RGBA8Sint,         u8"RGBA8Sint",         FormatType::Sint,      4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                ResourceFormat::RG16Float,         u8"RG16Float",         FormatType::Float,     4,         2,                      {1, 1},            16,  16, 0,  0,  false,  false,    false,
                ResourceFormat::RG16Unorm,         u8"RG16Unorm",         FormatType::Unorm,     4,         2,                      {1, 1},            16,  16, 0,  0,  false,  false,    false,
                ResourceFormat::RG16Uint,          u8"RG16Uint",          FormatType::Uint,      4,         2,                      {1, 1},            16,  16, 0,  0,  false,  false,    false,
                ResourceFormat::RG16Snorm,         u8"RG16Snorm",         FormatType::Snorm,     4,         2,                      {1, 1},            16,  16, 0,  0,  false,  false,    false,
                ResourceFormat::RG16Sint,          u8"RG16Sint",          FormatType::Sint,      4,         2,                      {1, 1},            16,  16, 0,  0,  false,  false,    false,
                //Format                           Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                ResourceFormat::R32Float,          u8"R32Float",          FormatType::Float,     4,         1,                      {1, 1},            32,  0,  0,  0,  false,  false,    false,
                ResourceFormat::R32Uint,           u8"R32Uint",           FormatType::Uint,      4,         1,                      {1, 1},            32,  0,  0,  0,  false,  false,    false,
                ResourceFormat::R32Sint,           u8"R32Sint",           FormatType::Sint,      4,         1,                      {1, 1},            32,  0,  0,  0,  false,  false,    false,
                //Format                           Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                ResourceFormat::RG8Unorm,          u8"RG8Unorm",          FormatType::Unorm,     2,         2,                      {1, 1},            8,   8,  0,  0,  false,  false,    false,
                ResourceFormat::RG8Uint,           u8"RG8Uint",           FormatType::Uint,      2,         2,                      {1, 1},            8,   8,  0,  0,  false,  false,    false,
                ResourceFormat::RG8Snorm,          u8"RG8Snorm",          FormatType::Snorm,     2,         2,                      {1, 1},            8,   8,  0,  0,  false,  false,    false,
                ResourceFormat::RG8Sint,           u8"RG8Sint",           FormatType::Sint,      2,         2,                      {1, 1},            8,   8,  0,  0,  false,  false,    false,
                //Format                           Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                ResourceFormat::R16Float,          u8"R16Float",          FormatType::Float,     2,         1,                      {1, 1},            16,  0,  0,  0,  false,  false,    false,
                ResourceFormat::R16Unorm,          u8"R16Unorm",          FormatType::Unorm,     2,         1,                      {1, 1},            16,  0,  0,  0,  false,  false,    false,
                ResourceFormat::R16Uint,           u8"R16Uint",           FormatType::Uint,      2,         1,                      {1, 1},            16,  0,  0,  0,  false,  false,    false,
                ResourceFormat::R16Snorm,          u8"R16Snorm",          FormatType::Snorm,     2,         1,                      {1, 1},            16,  0,  0,  0,  false,  false,    false,
                ResourceFormat::R16Sint,           u8"R16Sint",           FormatType::Sint,      2,         1,                      {1, 1},            16,  0,  0,  0,  false,  false,    false,
                ResourceFormat::R8Unorm,           u8"R8Unorm",           FormatType::Unorm,     1,         1,                      {1, 1},            8,   0,  0,  0,  false,  false,    false,
                ResourceFormat::R8Uint,            u8"R8Uint",            FormatType::Uint,      1,         1,                      {1, 1},            8,   0,  0,  0,  false,  false,    false,
                ResourceFormat::R8Snorm,           u8"R8Snorm",           FormatType::Snorm,     1,         1,                      {1, 1},            8,   0,  0,  0,  false,  false,    false,
                ResourceFormat::R8Sint,            u8"R8Sint",            FormatType::Sint,      1,         1,                      {1, 1},            8,   0,  0,  0,  false,  false,    false,
                ResourceFormat::A8Unorm,           u8"A8Unorm",           FormatType::Unorm,     1,         1,                      {1, 1},            0,   0,  0,  8,  false,  false,    false,
                //Format                           Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                ResourceFormat::D32FloatS8X24Uint, u8"D32FloatS8X24Uint", FormatType::Float,     8,         2,                      {1, 1},            32,  8,  24, 0,  true,   true,     false,
                ResourceFormat::D32Float,          u8"D32Float",          FormatType::Float,     4,         1,                      {1, 1},            32,  0,  0,  0,  true,   false,    false,
                ResourceFormat::D24UnormS8Uint,    u8"D24UnormS8Uint",    FormatType::Unorm,     4,         2,                      {1, 1},            24,  8,  0,  0,  true,   true,     false,
                ResourceFormat::D16Unorm,          u8"D16Unorm",          FormatType::Unorm,     2,         1,                      {1, 1},            16,  0,  0,  0,  true,   false,    false,
                //Format                           Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                ResourceFormat::R32FloatX8X24,     u8"R32FloatX8X24",     FormatType::Float,     8,         2,                      {1, 1},            32,  8, 24,  0,  false,  false,    false,
                ResourceFormat::X32G8Uint,         u8"X32G8Uint",         FormatType::Uint,      8,         2,                      {1, 1},            32,  8,  0,  0,  false,  false,    false,
                ResourceFormat::R24UnormX8,        u8"R24UnormX8",        FormatType::Unorm,     4,         2,                      {1, 1},            24,  8,  0,  0,  false,  false,    false,
                ResourceFormat::X24G8Uint,         u8"X24G8Uint",         FormatType::Uint,      4,         2,                      {1, 1},            24,  8,  0,  0,  false,  false,    false,
                //Format                           Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                ResourceFormat::BC1Unorm,          u8"BC1Unorm",          FormatType::Unorm,     8,         3,                      {4, 4},            64,  0,  0,  0,  false,  false,    true,
                ResourceFormat::BC1UnormSrgb,      u8"BC1UnormSrgb",      FormatType::UnormSrgb, 8,         3,                      {4, 4},            64,  0,  0,  0,  false,  false,    true,
                ResourceFormat::BC2Unorm,          u8"BC2Unorm",          FormatType::Unorm,     16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                ResourceFormat::BC2UnormSrgb,      u8"BC2UnormSrgb",      FormatType::UnormSrgb, 16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                ResourceFormat::BC3Unorm,          u8"BC3Unorm",          FormatType::Unorm,     16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                ResourceFormat::BC3UnormSrgb,      u8"BC3UnormSrgb",      FormatType::UnormSrgb, 16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                ResourceFormat::BC4Unorm,          u8"BC4Unorm",          FormatType::Unorm,     8,         1,                      {4, 4},            64,  0,  0,  0,  false,  false,    true,
                ResourceFormat::BC4Snorm,          u8"BC4Snorm",          FormatType::Snorm,     8,         1,                      {4, 4},            64,  0,  0,  0,  false,  false,    true,
                ResourceFormat::BC5Unorm,          u8"BC5Unorm",          FormatType::Unorm,     16,        2,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                ResourceFormat::BC5Snorm,          u8"BC5Snorm",          FormatType::Snorm,     16,        2,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                ResourceFormat::BC6HU16,           u8"BC6HU16",           FormatType::Float,     16,        3,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                ResourceFormat::BC6HS16,           u8"BC6HS16",           FormatType::Float,     16,        3,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                ResourceFormat::BC7Unorm,          u8"BC7Unorm",          FormatType::Unorm,     16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                ResourceFormat::BC7UnormSrgb,      u8"BC7UnormSrgb",      FormatType::UnormSrgb, 16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                //Format                           Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                ResourceFormat::RGB16Float,        u8"RGB16Float",        FormatType::Float,     6,         3,                      {1, 1},            16,  16, 16, 0,  false,  false,    false,
                ResourceFormat::RGB16Unorm,        u8"RGB16Unorm",        FormatType::Unorm,     6,         3,                      {1, 1},            16,  16, 16, 0,  false,  false,    false,
                ResourceFormat::RGB16Uint,         u8"RGB16Uint",         FormatType::Uint,      6,         3,                      {1, 1},            16,  16, 16, 0,  false,  false,    false,
                ResourceFormat::RGB16Snorm,        u8"RGB16Snorm",        FormatType::Snorm,     6,         3,                      {1, 1},            16,  16, 16, 0,  false,  false,    false,
                ResourceFormat::RGB16Sint,         u8"RGB16Sint",         FormatType::Sint,      6,         3,                      {1, 1},            16,  16, 16, 0,  false,  false,    false,
                 //Format                          Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                ResourceFormat::RGB5A1Unorm,       u8"RGB5A1Unorm",       FormatType::Unorm,     2,         4,                      {1, 1},            5,   5,  5,  1,  false,  false,    false,
                ResourceFormat::RGB9E5Float,       u8"RGB9E5Float",       FormatType::Float,     4,         3,                      {1, 1},            9,   9,  9,  5,  false,  false,    false,
                //Format                           Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                ResourceFormat::BGRA8Unorm,        u8"BGRA8Unorm",        FormatType::Unorm,     4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                ResourceFormat::BGRA8UnormSrgb,    u8"BGRA8UnormSrgb",    FormatType::UnormSrgb, 4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                ResourceFormat::BGRX8Unorm,        u8"BGRX8Unorm",        FormatType::Unorm,     4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                ResourceFormat::BGRX8UnormSrgb,    u8"BGRX8UnormSrgb",    FormatType::UnormSrgb, 4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                //Format                           Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                ResourceFormat::R5G6B5Unorm,       u8"R5G6B5Unorm",       FormatType::Unorm,     2,         3,                      {1, 1},            5,   6,  5,  0,  false,  false,    false,
                ResourceFormat::Alpha32Float,      u8"Alpha32Float",      FormatType::Float,     4,         1,                      {1, 1},            0,   0,  0,  32, false,  false,    false,
        };
            // clang-format on

            static_assert(std::is_same<std::underlying_type<ResourceFormat>::type, uint32_t>::value);
            static_assert(std::size(formatInfo) == static_cast<uint32_t>(ResourceFormat::Count));
        }

        namespace ResourceFormatInfo
        {
            bool IsDepth(ResourceFormat format)
            {
                ASSERT(format == formatInfo[static_cast<uint32_t>(format)].format)

                return formatInfo[static_cast<uint32_t>(format)].isDepth;
            }

            bool IsStencil(ResourceFormat format)
            {
                ASSERT(format == formatInfo[static_cast<uint32_t>(format)].format)

                return formatInfo[static_cast<uint32_t>(format)].isStencil;
            }

            bool IsCompressed(ResourceFormat format)
            {
                ASSERT(format == formatInfo[static_cast<uint32_t>(format)].format)

                return formatInfo[static_cast<uint32_t>(format)].isCompressed;
            }

            uint32_t GetBlockSize(ResourceFormat format)
            {
                ASSERT(format == formatInfo[static_cast<uint32_t>(format)].format)

                return formatInfo[static_cast<uint32_t>(format)].blockSize;
            }

            uint32_t GetCompressionBlockWidth(ResourceFormat format)
            {
                ASSERT(format == formatInfo[static_cast<uint32_t>(format)].format)

                return formatInfo[static_cast<uint32_t>(format)].compressionBlock.width;
            }

            uint32_t GetCompressionBlockHeight(ResourceFormat format)
            {
                ASSERT(format == formatInfo[static_cast<uint32_t>(format)].format)

                return formatInfo[static_cast<uint32_t>(format)].compressionBlock.height;
            }

            U8String ToString(ResourceFormat format)
            {
                ASSERT(format == formatInfo[static_cast<uint32_t>(format)].format)

                return formatInfo[static_cast<uint32_t>(format)].name;
            }
        }

        template <>
        std::shared_ptr<Texture> Resource::GetTyped<Texture>()
        {
            ASSERT(resourceType_ == ResourceType::Texture)
            //TODO inherit_shared_from_this
            return std::static_pointer_cast<Texture>(shared_from_this());
        }
        /*
        template <>
        inline std::shared_ptr<Buffer> Resource::GetTyped<Buffer>()
        {
            ASSERT(resourceType_ == Type::Buffer)
            return std::dynamic_pointer_cast<Buffer>(shared_from_this());
        }*/
    }
}