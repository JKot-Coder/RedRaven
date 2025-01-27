#pragma once

#if defined(_MSC_BUILD) && !defined(__clang__) && !defined(__INTEL_COMPILER)
#define MSVC
#endif

#ifdef MSVC
#define FUNCTION_SIGNATURE __FUNCSIG__
#else
#define FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#endif

#include "ecs/Index.hpp"
#include "ecs/Hash.hpp"

namespace RR::Ecs
{
    struct TypeId : public Index<TypeId, HashType>{};

    template <typename T>
    struct TypeName
    {
    private:
        // use std::string_view since eastl::string_view::find are not constexpr
        constexpr static const eastl::string_view extractTypeNameFromSignature(std::string_view funcName) noexcept
        {
            const auto trimPrefix = [](std::string_view str, std::string_view trim) {
                return str.substr(str.find(trim) + trim.size());
            };

#ifdef MSVC
            funcName = trimPrefix(funcName, "RR::Ecs::TypeName<");
            auto end_pos = funcName.rfind(">::getTypeNameView(void) noexcept");
#else
            funcName = trimPrefix(funcName, "T = ");
            auto end_pos = funcName.find(";");
            if (end_pos == funcName.npos) { end_pos = funcName.find("]"); }
#endif
            funcName = funcName.substr(0, end_pos);
            return {funcName.data(), funcName.size()};
        }

        constexpr static eastl::string_view getTypeNameView() noexcept
        {
            return extractTypeNameFromSignature(FUNCTION_SIGNATURE);
        }

        template <int Size>
        struct FixedString
        {
            constexpr std::string_view c_str() const noexcept { return data.data(); };
            constexpr std::string_view string_view() const noexcept { return {data.data(), size}; };

            size_t size = 0;
            eastl::array<char, Size> data {};
        };

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
                constexpr eastl::string_view classString = "class ";
                if (substr.starts_with(classString))
                {
                    readIndex += classString.size();
                    continue;
                }

                constexpr eastl::string_view structString = "struct ";
                if (substr.starts_with(structString))
                {
                    readIndex += structString.size();
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

        static constexpr FixedTypeName name = filterTypeName(typeNameView);
    public:
        static constexpr std::string_view string_view() { return name.string_view(); }
        static constexpr char* c_str() { return name.c_str(); }
    };

    struct TypeDescriptor
    {
        TypeId id;
        uint32_t size;
        uint32_t alignment;
    };

    template <typename T>
    struct TypeTraits
    {
        static constexpr std::string_view Name = TypeName<T>::string_view();
        static constexpr uint64_t Hash = Ecs::ConstexprHash(Name);
        static constexpr TypeId Id = TypeId(Hash);

        static constexpr TypeDescriptor descriptor = {Id, sizeof(T), alignof(T)};
    };
}

#undef FUNCTION_SIGNATURE
#undef MSVC

namespace eastl
{
    using namespace RR::Ecs;
    template<>
    struct hash<TypeId> : eastl::hash<RR::Ecs::Index<TypeId, size_t>> {};
}
