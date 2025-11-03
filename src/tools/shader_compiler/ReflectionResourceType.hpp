#pragma once

/**
 * Define enum information. This is expected to be used as follows:
 *
 * enum class Foo { A, B, C };
 * ENUM_INFO(Foo, {
 *     { Foo::A, "A" },
 *     { Foo::B, "B" },
 *     { Foo::C, "C" },
 * })
 */
#define ENUM_INFO(T, ...)                                             \
    struct T##_info                                                   \
    {                                                                 \
        static eastl::span<eastl::pair<T, const char*>> items()       \
        {                                                             \
            static eastl::pair<T, const char*> items[] = __VA_ARGS__; \
            return {eastl::begin(items), eastl::end(items)};          \
        }                                                             \
    }

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