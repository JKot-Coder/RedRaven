#include "Resource.hpp"

#include "gapi/Texture.hpp"
//#include "gapi/Buffer.hpp"

namespace OpenDemo
{
    namespace Render
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
                Resource::Format format;
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
                //Format                             Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w] isDepth isStencil isCompressed
                Resource::Format::Unknown,           u8"Unknown",           FormatType::Unknown,   0,         0,                      {1, 1},            0,   0,  0,  0,  false,  false,    false,
                Resource::Format::RGBA32Float,       u8"RGBA32Float",       FormatType::Float,     16,        4,                      {1, 1},            32,  32, 32, 32, false,  false,    false,
                Resource::Format::RGBA32Uint,        u8"RGBA32Uint",        FormatType::Uint,      16,        4,                      {1, 1},            32,  32, 32, 32, false,  false,    false,
                Resource::Format::RGBA32Sint,        u8"RGBA32Sint",        FormatType::Sint,      16,        4,                      {1, 1},            32,  32, 32, 32, false,  false,    false,
                Resource::Format::RGB32Float,        u8"RGB32Float",        FormatType::Float,     12,        3,                      {1, 1},            32,  32, 32, 0,  false,  false,    false,
                Resource::Format::RGB32Uint,         u8"RGB32Uint",         FormatType::Uint,      12,        3,                      {1, 1},            32,  32, 32, 0,  false,  false,    false,
                Resource::Format::RGB32Sint,         u8"RGB32Sint",         FormatType::Sint,      12,        3,                      {1, 1},            32,  32, 32, 0,  false,  false,    false,
                Resource::Format::RGBA16Float,       u8"RGBA16Float",       FormatType::Float,     8,         4,                      {1, 1},            16,  16, 16, 16, false,  false,    false,
                Resource::Format::RGBA16Unorm,       u8"RGBA16Unorm",       FormatType::Unorm,     8,         4,                      {1, 1},            16,  16, 16, 16, false,  false,    false,
                Resource::Format::RGBA16Uint,        u8"RGBA16Uint",        FormatType::Uint,      8,         4,                      {1, 1},            16,  16, 16, 16, false,  false,    false,
                Resource::Format::RGBA16Snorm,       u8"RGBA16Snorm",       FormatType::Snorm,     8,         4,                      {1, 1},            16,  16, 16, 16, false,  false,    false,  
                Resource::Format::RGBA16Sint,        u8"RGBA16Sint",        FormatType::Sint,      8,         4,                      {1, 1},            16,  16, 16, 16, false,  false,    false,
                Resource::Format::RG32Float,         u8"RG32Float",         FormatType::Float,     8,         2,                      {1, 1},            32,  32, 0,  0,  false,  false,    false,
                Resource::Format::RG32Uint,          u8"RG32Uint",          FormatType::Uint,      8,         2,                      {1, 1},            32,  32, 0,  0,  false,  false,    false,
                Resource::Format::RG32Sint,          u8"RG32Sint",          FormatType::Sint,      8,         2,                      {1, 1},            32,  32, 0,  0,  false,  false,    false,
                //Format                             Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                Resource::Format::RGB10A2Unorm,      u8"RGB10A2Unorm",      FormatType::Unorm,     4,         4,                      {1, 1},            10,  10, 10, 2,  false,  false,    false,
                Resource::Format::RGB10A2Uint,       u8"RGB10A2Uint",       FormatType::Uint,      4,         4,                      {1, 1},            10,  10, 10, 2,  false,  false,    false,
                Resource::Format::R11G11B10Float,    u8"R11G11B10Float",    FormatType::Float,     4,         3,                      {1, 1},            11,  11, 10, 0,  false,  false,    false,
                Resource::Format::RGBA8Unorm,        u8"RGBA8Unorm",        FormatType::Unorm,     4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                Resource::Format::RGBA8UnormSrgb,    u8"RGBA8UnormSrgb",    FormatType::UnormSrgb, 4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                Resource::Format::RGBA8Uint,         u8"RGBA8Uint",         FormatType::Uint,      4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                Resource::Format::RGBA8Snorm,        u8"RGBA8Snorm",        FormatType::Snorm,     4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                Resource::Format::RGBA8Sint,         u8"RGBA8Sint",         FormatType::Sint,      4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                Resource::Format::RG16Float,         u8"RG16Float",         FormatType::Float,     4,         2,                      {1, 1},            16,  16, 0,  0,  false,  false,    false,
                Resource::Format::RG16Unorm,         u8"RG16Unorm",         FormatType::Unorm,     4,         2,                      {1, 1},            16,  16, 0,  0,  false,  false,    false,
                Resource::Format::RG16Uint,          u8"RG16Uint",          FormatType::Uint,      4,         2,                      {1, 1},            16,  16, 0,  0,  false,  false,    false,
                Resource::Format::RG16Snorm,         u8"RG16Snorm",         FormatType::Snorm,     4,         2,                      {1, 1},            16,  16, 0,  0,  false,  false,    false,
                Resource::Format::RG16Sint,          u8"RG16Sint",          FormatType::Sint,      4,         2,                      {1, 1},            16,  16, 0,  0,  false,  false,    false,
                //Format                             Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                Resource::Format::R32Float,          u8"R32Float",          FormatType::Float,     4,         1,                      {1, 1},            32,  0,  0,  0,  false,  false,    false,
                Resource::Format::R32Uint,           u8"R32Uint",           FormatType::Uint,      4,         1,                      {1, 1},            32,  0,  0,  0,  false,  false,    false,
                Resource::Format::R32Sint,           u8"R32Sint",           FormatType::Sint,      4,         1,                      {1, 1},            32,  0,  0,  0,  false,  false,    false,
                //Format                             Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                Resource::Format::RG8Unorm,          u8"RG8Unorm",          FormatType::Unorm,     2,         2,                      {1, 1},            8,   8,  0,  0,  false,  false,    false,
                Resource::Format::RG8Uint,           u8"RG8Uint",           FormatType::Uint,      2,         2,                      {1, 1},            8,   8,  0,  0,  false,  false,    false,
                Resource::Format::RG8Snorm,          u8"RG8Snorm",          FormatType::Snorm,     2,         2,                      {1, 1},            8,   8,  0,  0,  false,  false,    false,
                Resource::Format::RG8Sint,           u8"RG8Sint",           FormatType::Sint,      2,         2,                      {1, 1},            8,   8,  0,  0,  false,  false,    false,
                //Format                             Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                Resource::Format::R16Float,          u8"R16Float",          FormatType::Float,     2,         1,                      {1, 1},            16,  0,  0,  0,  false,  false,    false,
                Resource::Format::R16Unorm,          u8"R16Unorm",          FormatType::Unorm,     2,         1,                      {1, 1},            16,  0,  0,  0,  false,  false,    false,
                Resource::Format::R16Uint,           u8"R16Uint",           FormatType::Uint,      2,         1,                      {1, 1},            16,  0,  0,  0,  false,  false,    false,
                Resource::Format::R16Snorm,          u8"R16Snorm",          FormatType::Snorm,     2,         1,                      {1, 1},            16,  0,  0,  0,  false,  false,    false,
                Resource::Format::R16Sint,           u8"R16Sint",           FormatType::Sint,      2,         1,                      {1, 1},            16,  0,  0,  0,  false,  false,    false,
                Resource::Format::R8Unorm,           u8"R8Unorm",           FormatType::Unorm,     1,         1,                      {1, 1},            8,   0,  0,  0,  false,  false,    false,
                Resource::Format::R8Uint,            u8"R8Uint",            FormatType::Uint,      1,         1,                      {1, 1},            8,   0,  0,  0,  false,  false,    false,
                Resource::Format::R8Snorm,           u8"R8Snorm",           FormatType::Snorm,     1,         1,                      {1, 1},            8,   0,  0,  0,  false,  false,    false,
                Resource::Format::R8Sint,            u8"R8Sint",            FormatType::Sint,      1,         1,                      {1, 1},            8,   0,  0,  0,  false,  false,    false,
                Resource::Format::A8Unorm,           u8"A8Unorm",           FormatType::Unorm,     1,         1,                      {1, 1},            0,   0,  0,  8,  false,  false,    false,
                //Format                             Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                Resource::Format::D32FloatS8X24Uint, u8"D32FloatS8X24Uint", FormatType::Float,     8,         2,                      {1, 1},            32,  8,  24, 0,  true,   true,     false,
                Resource::Format::D32Float,          u8"D32Float",          FormatType::Float,     4,         1,                      {1, 1},            32,  0,  0,  0,  true,   false,    false,
                Resource::Format::D24UnormS8Uint,    u8"D24UnormS8Uint",    FormatType::Unorm,     4,         2,                      {1, 1},            24,  8,  0,  0,  true,   true,     false,
                Resource::Format::D16Unorm,          u8"D16Unorm",          FormatType::Unorm,     2,         1,                      {1, 1},            16,  0,  0,  0,  true,   false,    false,
                //Format                             Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                Resource::Format::R32FloatX8X24,     u8"R32FloatX8X24",     FormatType::Float,     8,         2,                      {1, 1},            32,  8, 24,  0,  false,  false,    false,
                Resource::Format::X32G8Uint,         u8"X32G8Uint",         FormatType::Uint,      8,         2,                      {1, 1},            32,  8,  0,  0,  false,  false,    false,
                Resource::Format::R24UnormX8,        u8"R24UnormX8",        FormatType::Unorm,     4,         2,                      {1, 1},            24,  8,  0,  0,  false,  false,    false,
                Resource::Format::X24G8Uint,         u8"X24G8Uint",         FormatType::Uint,      4,         2,                      {1, 1},            24,  8,  0,  0,  false,  false,    false,
                //Format                             Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                Resource::Format::BC1Unorm,          u8"BC1Unorm",          FormatType::Unorm,     8,         3,                      {4, 4},            64,  0,  0,  0,  false,  false,    true,
                Resource::Format::BC1UnormSrgb,      u8"BC1UnormSrgb",      FormatType::UnormSrgb, 8,         3,                      {4, 4},            64,  0,  0,  0,  false,  false,    true,
                Resource::Format::BC2Unorm,          u8"BC2Unorm",          FormatType::Unorm,     16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                Resource::Format::BC2UnormSrgb,      u8"BC2UnormSrgb",      FormatType::UnormSrgb, 16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                Resource::Format::BC3Unorm,          u8"BC3Unorm",          FormatType::Unorm,     16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                Resource::Format::BC3UnormSrgb,      u8"BC3UnormSrgb",      FormatType::UnormSrgb, 16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                Resource::Format::BC4Unorm,          u8"BC4Unorm",          FormatType::Unorm,     8,         1,                      {4, 4},            64,  0,  0,  0,  false,  false,    true,
                Resource::Format::BC4Snorm,          u8"BC4Snorm",          FormatType::Snorm,     8,         1,                      {4, 4},            64,  0,  0,  0,  false,  false,    true,
                Resource::Format::BC5Unorm,          u8"BC5Unorm",          FormatType::Unorm,     16,        2,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                Resource::Format::BC5Snorm,          u8"BC5Snorm",          FormatType::Snorm,     16,        2,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                Resource::Format::BC6HU16,           u8"BC6HU16",           FormatType::Float,     16,        3,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                Resource::Format::BC6HS16,           u8"BC6HS16",           FormatType::Float,     16,        3,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                Resource::Format::BC7Unorm,          u8"BC7Unorm",          FormatType::Unorm,     16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                Resource::Format::BC7UnormSrgb,      u8"BC7UnormSrgb",      FormatType::UnormSrgb, 16,        4,                      {4, 4},            128, 0,  0,  0,  false,  false,    true,
                //Format                             Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                Resource::Format::RGB16Float,        u8"RGB16Float",        FormatType::Float,     6,         3,                      {1, 1},            16,  16, 16, 0,  false,  false,    false,
                Resource::Format::RGB16Unorm,        u8"RGB16Unorm",        FormatType::Unorm,     6,         3,                      {1, 1},            16,  16, 16, 0,  false,  false,    false,
                Resource::Format::RGB16Uint,         u8"RGB16Uint",         FormatType::Uint,      6,         3,                      {1, 1},            16,  16, 16, 0,  false,  false,    false,
                Resource::Format::RGB16Snorm,        u8"RGB16Snorm",        FormatType::Snorm,     6,         3,                      {1, 1},            16,  16, 16, 0,  false,  false,    false,
                Resource::Format::RGB16Sint,         u8"RGB16Sint",         FormatType::Sint,      6,         3,                      {1, 1},            16,  16, 16, 0,  false,  false,    false,
                 //Format                             Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                Resource::Format::RGB5A1Unorm,       u8"RGB5A1Unorm",       FormatType::Unorm,     2,         4,                      {1, 1},            5,   5,  5,  1,  false,  false,    false,
                Resource::Format::RGB9E5Float,       u8"RGB9E5Float",       FormatType::Float,     4,         3,                      {1, 1},            9,   9,  9,  5,  false,  false,    false,
                //Format                             Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed         
                Resource::Format::BGRA8Unorm,        u8"BGRA8Unorm",        FormatType::Unorm,     4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                Resource::Format::BGRA8UnormSrgb,    u8"BGRA8UnormSrgb",    FormatType::UnormSrgb, 4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                Resource::Format::BGRX8Unorm,        u8"BGRX8Unorm",        FormatType::Unorm,     4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                Resource::Format::BGRX8UnormSrgb,    u8"BGRX8UnormSrgb",    FormatType::UnormSrgb, 4,         4,                      {1, 1},            8,   8,  8,  8,  false,  false,    false,
                //Format                             Name                   Type               BlockSize ChannelCount CompressionBlock{w, h} ChannelBits[x,   y,  z,  w]  isDepth isStencil isCompressed
                Resource::Format::R5G6B5Unorm,       u8"R5G6B5Unorm",       FormatType::Unorm,     2,         3,                      {1, 1},            5,   6,  5,  0,  false,  false,    false,
                Resource::Format::Alpha32Float,      u8"Alpha32Float",      FormatType::Float,     4,         1,                      {1, 1},            0,   0,  0,  32, false,  false,    false,
        };
            // clang-format on

            static_assert(std::size(formatInfo) == Resource::Format::Count);
        }

        namespace Private
        {
            bool ResourceFormat::IsDepth() const
            {
                ASSERT(IsValid())
                ASSERT(value_ == formatInfo[value_].format)

                return formatInfo[value_].isDepth;
            }

            bool ResourceFormat::IsStencil() const
            {
                ASSERT(IsValid())
                ASSERT(value_ == formatInfo[value_].format)

                return formatInfo[value_].isStencil;
            }

            bool Resource::Format::IsCompressed() const
            {
                ASSERT(IsValid())
                ASSERT(value_ == formatInfo[value_].format)

                return formatInfo[value_].isCompressed;
            }

            uint32_t Resource::Format::GetBlockSize() const
            {
                ASSERT(IsValid())
                ASSERT(value_ == formatInfo[value_].format)

                return formatInfo[value_].blockSize;
            }

            uint32_t ResourceFormat::GetCompressionBlockWidth() const
            {
                ASSERT(IsValid())
                ASSERT(value_ == formatInfo[value_].format)

                return formatInfo[value_].compressionBlock.width;
            }

            uint32_t ResourceFormat::GetCompressionBlockHeight() const
            {
                ASSERT(IsValid())
                ASSERT(value_ == formatInfo[value_].format)

                return formatInfo[value_].compressionBlock.height;
            }

            U8String ResourceFormat::ToString() const
            {
                ASSERT(IsValid())
                ASSERT(value_ == formatInfo[value_].format)

                return formatInfo[value_].name;
            }

        }

        template <>
        Texture& Resource::GetTyped<Texture>()
        {
            ASSERT(resourceType_ == Type::Texture)
            return dynamic_cast<Texture&>(*this);
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