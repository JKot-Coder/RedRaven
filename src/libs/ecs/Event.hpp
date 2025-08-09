#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/meta/ComponentTraits.hpp"

namespace RR::Ecs
{
    using EventId = Index<struct EventIdTag, HashType>;

    template <typename T>
    inline constexpr EventId GetEventId = EventId(GetComponentId<T>.GetRaw());

    struct Event
    {
        using SizeType = uint16_t;

        constexpr Event(EventId id, size_t size) : id(id), size(static_cast<SizeType>(size)) {
            ASSERT(size < (size_t)std::numeric_limits<SizeType>::max);
        };

        template <typename T>
        bool Is() const
        {
            static_assert(eastl::is_base_of_v<Event, T>, "T should be derived from Event");
            return eastl::is_same_v<T, Event> || GetEventId<T> == id;
        }

        template <typename T>
        T& As()
        {
            static_assert(eastl::is_base_of_v<Event, T>, "T should be derived from Event");
            ASSERT(Is<T>());
            return *static_cast<T*>(this);
        }

        template <typename T>
        const T& As() const
        {
            static_assert(eastl::is_base_of_v<Event, T>, "T should be derived from Event");
            ASSERT(Is<T>());
            return *static_cast<const T*>(this);
        }

        template <typename T>
        T* TryAs()
        {
            static_assert(eastl::is_base_of_v<Event, T>, "T should be derived from Event");
            return Is<T>() ? static_cast<T*>(this) : nullptr;
        }

        template <typename T>
        const T* TryAs() const
        {
            static_assert(eastl::is_base_of_v<Event, T>, "T should be derived from Event");
            return Is<T>() ? static_cast<const T*>(this) : nullptr;
        }

        EventId id;
        SizeType size;
    };

    struct OnAppear : Event
    {
        OnAppear() : Event(GetEventId<OnAppear>, sizeof(OnAppear)) { };
    };

    struct OnDissapear : Event
    {
        OnDissapear() : Event(GetEventId<OnDissapear>, sizeof(OnDissapear)) { };
    };

    struct OnChange : Event
    {
        OnChange() : Event(GetEventId<OnChange>, sizeof(OnChange)) { };
    };
}