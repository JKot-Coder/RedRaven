#pragma once

#include "ecs/Hash.hpp"
#include "ecs/Index.hpp"
#include "ecs/TypeTraits.hpp"
#include <EASTL/type_traits.h>

namespace RR::Ecs
{
    using ComponentId = Index<struct ComponentIdTag, HashType>;

    struct ComponentInfo
    {
        ComponentId id;
        size_t size;
        size_t alignment;
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
        static constexpr ComponentInfo ComponentInfo{Id, Size, Alignment};
        static constexpr auto Name = GetTypeName<RawType>;
    };

    template <typename T>
    using GetComponentType = typename ComponentType<T>::Type;

    template <typename T>
    static constexpr ComponentId GetComponentId = ComponentTraits<T>::Id;

    template <typename T>
    static constexpr ComponentInfo GetComponentInfo = ComponentTraits<T>::ComponentInfo;

    template <typename T>
    static constexpr auto GetComponentName = ComponentTraits<T>::Name;
}