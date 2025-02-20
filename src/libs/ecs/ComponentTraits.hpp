#pragma once

#include "ecs/Hash.hpp"
#include "ecs/Index.hpp"
#include "ecs/TypeTraits.hpp"
#include <EASTL/fixed_vector.h>
#include <EASTL/type_traits.h>
#include <EASTL/vector_set.h>

namespace RR::Ecs
{
    using ComponentId = Index<struct ComponentIdTag, HashType>;
    template <typename Key, size_t ElementsCount, bool EnableOverflow = true>
    using FixedVectorSet = eastl::vector_set<Key, eastl::less<Key>, EASTLAllocatorType, eastl::fixed_vector<Key, ElementsCount, EnableOverflow>>;
    using ComponentsSet = FixedVectorSet<ComponentId, 32>;
    struct ComponentsView
    {
        using Iterator = const ComponentId*;
        ComponentsView(Iterator begin, Iterator end) : begin_(begin), end_(end) { }

        Iterator begin() const { return begin_; }
        Iterator end() const { return end_; }

    private:
        Iterator begin_;
        Iterator end_;
    };

    namespace details
    {
        template <typename T>
        void DefaultConstructor(void* data)
        {
            if constexpr (std::is_default_constructible<T>::value &&
                          !std::is_trivially_default_constructible<T>::value)
            {
                new (data) T();
            }
            else
                static_assert("T is not default constructible");
        }

        template <typename T>
        void Destructor(void* data)
        {
            ((T*)data)->~T();
        }

        template <typename T>
        static void CopyConstructor([[maybe_unused]] void* dest, [[maybe_unused]] const void* source)
        {
            if constexpr (eastl::is_copy_constructible_v<T>)
            {
                new (dest) T(*(T*)source);
            }
            else
                static_assert("T is not copy constructible");
        }

        template <typename T>
        static void MoveConstructor([[maybe_unused]] void* dest, [[maybe_unused]] void* source)
        {
            if constexpr (eastl::is_move_constructible_v<T>)
            {
                new (dest) T(eastl::move(*(T*)source));
            }
            else
                static_assert("T is not move constructible");
        }

    } // details

    struct ComponentInfo
    {
        using DefaultConstructor = void (*)(void* mem);
        using Destructor = void (*)(void* mem);
        using CopyConstructor = void (*)(void* dest, const void* src);
        using MoveConstructor = void (*)(void* dest, void* src);

        ComponentId id;
        size_t size;
        size_t alignment;
        DefaultConstructor constructDefault;
        Destructor destructor;
        CopyConstructor copy;
        MoveConstructor move;

        template <bool Condition, typename T>
        static constexpr T eval(T&& v)
        {
            if constexpr (Condition)
            {
                return nullptr;
            }
            else
                return eastl::forward<T>(v);
        }

        template <typename T>
        static constexpr ComponentInfo Create()
        {
            return
            {
                ComponentId(GetTypeId<T>.Value()),
                    sizeof(T),
                    alignof(T),
                    eval<eastl::is_trivially_default_constructible_v<T>>(&details::DefaultConstructor<T>),
                    eval<eastl::is_trivially_destructible_v<T>>(&details::Destructor<T>),
                    eval<eastl::is_trivially_copy_constructible_v<T>>(&details::CopyConstructor<T>),
                    eval<eastl::is_trivially_move_constructible_v<T>>(&details::MoveConstructor<T>)
            };
        }
    };

    template <typename T>
    class ComponentType
    {
        // Remove reference, pointer, and const qualifiers in sequence
        using NoCvRef = typename eastl::remove_cv<typename eastl::remove_reference<T>::type>::type;
        using NoCvRefPtr = typename eastl::remove_cv<typename eastl::remove_pointer<NoCvRef>::type>::type;

    public:
        using Type = NoCvRefPtr;
    };

    template <typename T>
    struct ComponentTraits
    {
        using RawType = typename ComponentType<T>::Type;

        static constexpr ComponentId Id = ComponentId(GetTypeId<RawType>.Value());

        static constexpr size_t Size = sizeof(RawType);
        static constexpr size_t Alignment = alignof(RawType);
        static constexpr ComponentInfo ComponentInfo = ComponentInfo::Create<T>();
    };

    template <typename T>
    using GetComponentType = typename ComponentType<T>::Type;

    template <typename T>
    static constexpr ComponentId GetComponentId = ComponentTraits<T>::Id;

    template <typename T>
    static constexpr ComponentInfo GetComponentInfo = ComponentTraits<T>::ComponentInfo;
}