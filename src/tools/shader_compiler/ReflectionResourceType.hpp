#pragma once

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
    };
}