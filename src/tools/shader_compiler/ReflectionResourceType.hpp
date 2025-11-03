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
    // Helper using ADL to find EnumInfo in other namespaces.
    template <typename T>
    using EnumInfo = decltype(falcorFindEnumInfoADL(std::declval<T>()));

    template <typename, typename = void>
    struct has_enum_info : std::false_type
    {
    };

    template <typename T>
    struct has_enum_info<T, std::void_t<decltype(EnumInfo<T>::items)>> : std::true_type
    {
    };

    template <typename T>
    inline constexpr bool has_enum_info_v = has_enum_info<T>::value;

    /**
     * Convert an enum value to a string.
     * Throws if the enum value is not found in the registered enum information.
     */
    template <typename T, eastl::enable_if_t<has_enum_info_v<T>, bool> = true>
    inline const char* enumToString(T value)
    {
        const auto& items = EnumInfo<T>::items();
        auto it = eastl::find_if(eastl::begin(items), eastl::end(items), [value](const auto& item) { return item.first == value; });
        if (it == items.end())
            THROW("Invalid enum value {}", int(value));
        return it->second;
    }

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
}