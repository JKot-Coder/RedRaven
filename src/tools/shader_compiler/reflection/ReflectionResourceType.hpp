#pragma once

#include "Utils.hpp"

namespace RR
{
    /**
     * Reflection object for resources
     */
    class ReflectionResourceType
    {
    public:
        /**
         * The type of the resource
         */
        enum class Type
        {
            Texture,
            StructuredBuffer,
            RawBuffer,
            TypedBuffer,
            Sampler,
            ConstantBuffer,
            AccelerationStructure,
        };
        ENUM_INFO(
            Type,
            {
                {Type::Texture, "Texture"},
                {Type::StructuredBuffer, "StructuredBuffer"},
                {Type::RawBuffer, "RawBuffer"},
                {Type::TypedBuffer, "TypedBuffer"},
                {Type::Sampler, "Sampler"},
                {Type::ConstantBuffer, "ConstantBuffer"},
                {Type::AccelerationStructure, "AccelerationStructure"},
            });

        /**
         * The resource dimension
         */
        enum class Dimensions
        {
            Unknown,
            Texture1D,
            Texture2D,
            Texture3D,
            TextureCube,
            Texture1DArray,
            Texture2DArray,
            Texture2DMS,
            Texture2DMSArray,
            TextureCubeArray,
            AccelerationStructure,
            Buffer,

            Count
        };
        ENUM_INFO(
            Dimensions,
            {
                {Dimensions::Unknown, "Unknown"},
                {Dimensions::Texture1D, "Texture1D"},
                {Dimensions::Texture2D, "Texture2D"},
                {Dimensions::Texture3D, "Texture3D"},
                {Dimensions::TextureCube, "TextureCube"},
                {Dimensions::Texture1DArray, "Texture1DArray"},
                {Dimensions::Texture2DArray, "Texture2DArray"},
                {Dimensions::Texture2DMS, "Texture2DMS"},
                {Dimensions::Texture2DMSArray, "Texture2DMSArray"},
                {Dimensions::TextureCubeArray, "TextureCubeArray"},
                {Dimensions::AccelerationStructure, "AccelerationStructure"},
                {Dimensions::Buffer, "Buffer"},
            });
    };

    ENUM_REGISTER(ReflectionResourceType::Type);
    ENUM_REGISTER(ReflectionResourceType::Dimensions);
}