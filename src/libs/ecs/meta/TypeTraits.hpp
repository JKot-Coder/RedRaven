#pragma once

#if defined(_MSC_BUILD) && !defined(__clang__) && !defined(__INTEL_COMPILER)
#define MSVC
#endif

#ifdef MSVC
#define FUNCTION_SIGNATURE __FUNCSIG__
#else
#define FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#endif

#include "ecs/EntityId.hpp"
#include "ecs/Hash.hpp"
#include "ecs/Index.hpp"
#include <EASTL/array.h>
#include <EASTL/string_view.h>

namespace RR::Ecs::Meta
{
    using TypeId = Index<struct TypeIdTag, HashType>;

    /**
     * @brief Compile-time type name extraction and storage
     */
    template <typename T>
    struct TypeName
    {
    private:
        template <size_t Size>
        struct FixedString
        {
            [[nodiscard]] constexpr std::string_view c_str() const noexcept { return data.data(); }
            [[nodiscard]] constexpr std::string_view string_view() const noexcept { return {data.data(), size}; }

        private:
            friend struct TypeName<T>;
            size_t size = 0;
            eastl::array<char, Size> data {};
        };

        // Extract type name from compiler-specific function signature
        static constexpr eastl::string_view extractTypeNameFromSignature(std::string_view funcName) noexcept
        {
            const auto trimPrefix = [](std::string_view str, std::string_view trim) {
                return str.substr(str.find(trim) + trim.size());
            };

#ifdef MSVC
            funcName = trimPrefix(funcName, "RR::Ecs::TypeName<");
            auto end_pos = funcName.rfind(">::getTypeNameView(void) noexcept");
            while (end_pos != 0 && funcName[end_pos - 1] == ' ')
                end_pos--;
#else
            funcName = trimPrefix(funcName, "T = ");
            auto end_pos = funcName.find(';');
            if (end_pos == eastl::string_view::npos)
                end_pos = funcName.find(']');
#endif

            return {funcName.data(), end_pos};
        }

        static constexpr eastl::string_view getTypeNameView() noexcept
        {
            return extractTypeNameFromSignature(FUNCTION_SIGNATURE);
        }

        static constexpr eastl::string_view typeNameView = getTypeNameView();
        using FixedTypeName = FixedString<typeNameView.size()>;

        constexpr static FixedTypeName filterTypeName(eastl::string_view typeName)
        {
            FixedTypeName result;
            auto& resultData = result.data;

            size_t readIndex = 0;
            size_t writeIndex = 0;

            while (readIndex < typeName.size() && writeIndex < resultData.size())
            {
#ifdef MSVC
                const auto substr = typeName.substr(readIndex);
                if (substr.starts_with("class "))
                {
                    readIndex += 6;
                    continue;
                }
                if (substr.starts_with("struct "))
                {
                    readIndex += 7;
                    continue;
                }
#endif

                resultData[writeIndex++] = typeName[readIndex++];
            }

            for (size_t i = writeIndex; i < resultData.size(); ++i)
                resultData[i] = '\0';

            result.size = writeIndex;
            return result;
        }

        static constexpr auto name = filterTypeName(typeNameView);

    public:
        static constexpr std::string_view string_view() noexcept { return name.string_view(); }
        static constexpr const char* c_str() noexcept { return name.c_str(); }
    };

    /**
     * @brief Type descriptor containing runtime type information
     */
    struct TypeDescriptor
    {
        constexpr TypeDescriptor(TypeId id, uint32_t size, uint32_t alignment)
            : id(id), size(size), alignment(alignment) {}

        TypeId id;
        uint32_t size;
        uint32_t alignment;
    };

    /**
     * @brief Compile-time type traits for ECS types
     */
    template <typename T>
    struct TypeTraits
    {
        static constexpr std::string_view Name = TypeName<T>::string_view();
        static constexpr HashType Hash = Ecs::ConstexprHash(Name);
        static constexpr TypeId Id = TypeId(Hash);

        static constexpr TypeDescriptor Descriptor() { return TypeDescriptor{Id, sizeof(T), alignof(T)}; }
    };

    // Specialization for EntityId to ensure it always has ID 0
    template <>
    struct TypeTraits<EntityId>
    {
        static constexpr std::string_view Name = TypeName<EntityId>::string_view();
        static constexpr HashType Hash = 0;
        static constexpr TypeId Id = TypeId(0);

        static constexpr TypeDescriptor Descriptor() { return TypeDescriptor{Id, sizeof(EntityId), alignof(EntityId)}; }
    };

    /**
     * @brief Helper type aliases for common operations
     */
    template <typename T>
    inline constexpr std::string_view GetTypeName = TypeTraits<T>::Name;

    template <typename T>
    inline constexpr HashType GetTypeHash = TypeTraits<T>::Hash;

    template <typename T>
    inline constexpr TypeId GetTypeId = TypeTraits<T>::Id;

    template <typename T>
    inline constexpr TypeDescriptor GetTypeDescriptor = TypeTraits<T>::Descriptor();
}

#undef FUNCTION_SIGNATURE
#undef MSVC
/*
namespace eastl
{
    using namespace RR::Ecs;
    template<>
    struct hash<TypeId> : eastl::hash<TypeId::ValueType> {};
}*/
