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
    constexpr size_t PreallocatedComponentsCount = 32;
    using ComponentsSet = FixedVectorSet<ComponentId, PreallocatedComponentsCount>;

    template <bool Sorted>
    struct ComponentsView
    {
        using Iterator = const ComponentId*;
        using IsSorted = std::integral_constant<bool, Sorted>;
        template <typename T>
        constexpr ComponentsView(const T& containter) : begin_(containter.begin()), end_(containter.end()) { }
        constexpr ComponentsView(Iterator begin, size_t count) : begin_(begin), end_(begin + count) { }
        constexpr ComponentsView(Iterator begin, Iterator end) : begin_(begin), end_(end) { }

        constexpr Iterator begin() const { return begin_; }
        constexpr Iterator end() const { return end_; }
        constexpr size_t size() const { return eastl::distance(begin_, end_); }

        template <bool S = Sorted, typename = std::enable_if_t<S>>
        constexpr bool IsIncludes(const ComponentsView& other) const
        {
            return std::includes(begin_, end_, other.begin_, other.end_);
        }

        template <bool S = Sorted, typename = std::enable_if_t<S>>
        constexpr bool IsIntersects(const ComponentsView& other) const
        {
            auto first1 = begin();
            auto last1 = end();
            auto first2 = other.begin();
            auto last2 = other.end();

            while (first1 != last1 && first2 != last2)
            {
                if (*first1 < *first2)
                    ++first1;
                else
                {
                    if (*first2 == *first1)
                        return true;
                    ++first2;
                }
            }
            return false;
        }

        template <bool S = Sorted, typename = std::enable_if_t<S>, typename Callback>
        void ForEachMissing(const ComponentsView& other, Callback&& callback)
        {
            auto first1 = begin();
            auto first2 = other.begin();
            auto last1 = end();
            auto last2 = other.end();

            while (first1 != last1 && first2 != last2)
            {
                if (*first1 < *first2)
                    eastl::invoke(eastl::forward<Callback>(callback), *first1++);
                else
                {
                    if (!(*first2 < *first1))
                        ++first1;
                    ++first2;
                }
            }
        }

    private:
        Iterator begin_;
        Iterator end_;
    };

    using UnsortedComponentsView = ComponentsView<false>;
    using SortedComponentsView = ComponentsView<true>;

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
    }

    struct ComponentInfo
    {
        using DefaultConstructor = void (*)(void* mem);
        using Destructor = void (*)(void* mem);
        using MoveOrCopy = void (*)(void* dest, void* src);

        ComponentId id;
        size_t size;
        bool isTrackable : 1;
        size_t alignment : 15;
        DefaultConstructor constructDefault;
        Destructor destructor;
        MoveOrCopy move;
        MoveOrCopy copy;

        bool operator==(const ComponentInfo& other) const
        {
            return id == other.id &&
                   isTrackable == other.isTrackable &&
                   size == other.size &&
                   alignment == other.alignment &&
                   constructDefault == other.constructDefault &&
                   destructor == other.destructor &&
                   move == other.move;
        }

        bool operator!=(const ComponentInfo& other) const
        {
            return !(*this == other);
        }

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
            return {
                ComponentId(GetTypeId<T>.GetRaw()),
                eastl::is_empty_v<T> ? 0 : sizeof(T),
                false,
                alignof(T),
                eval<eastl::is_trivially_default_constructible_v<T>>(&details::DefaultConstructor<T>),
                eval<eastl::is_trivially_destructible_v<T>>(&details::Destructor<T>),
                [](void* dest, void* source) {
                    if constexpr (std::is_move_constructible_v<T>)
                    {
                        new (dest) T(std::move(*static_cast<T*>(source)));
                    }
                    else
                        static_assert(sizeof(T) == 0, "Type T must be move constructible");
                },
                [](void* dest, void* source) {
                    if constexpr (std::is_copy_constructible_v<T>)
                    {
                        new (dest) T(*static_cast<T*>(source));
                    }
                    else
                        static_assert(sizeof(T) == 0, "Type T must be copy constructible");
                }
                };
        }
    };

    template <typename Storage>
    struct ComponentInfoIterator
    {
        using ComponentIdIterator = const ComponentId*;

        using value_type = ComponentInfo;
        using difference_type = std::ptrdiff_t;
        using pointer = ComponentInfo*;
        using reference = ComponentInfo&;
        using iterator_category = std::forward_iterator_tag;

        explicit ComponentInfoIterator(Storage& storage, ComponentIdIterator iterator) : storage(storage), iter(iterator) { }

        reference operator*() const
        {
            ComponentId id = *iter;
            auto it = storage.find(id);
            ASSERT(it != storage.end());
            return it->second;
        }

        pointer operator->() const
        {
            ComponentId id = *iter;
            auto it = storage.find(id);
            ASSERT(it != storage.end());
            return it->second;
        }

        ComponentInfoIterator& operator++()
        {
            ++iter;
            return *this;
        }

        ComponentInfoIterator operator++(int)
        {
            ComponentInfoIterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const ComponentInfoIterator& other) const { return iter == other.iter; }
        bool operator!=(const ComponentInfoIterator& other) const { return iter != other.iter; }

    private:
        Storage& storage;
        ComponentIdIterator iter;
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

        static constexpr ComponentId Id = ComponentId(GetTypeId<RawType>.GetRaw());

        static constexpr size_t Size = sizeof(RawType);
        static constexpr size_t Alignment = alignof(RawType);
        static constexpr ComponentInfo ComponentInfo = ComponentInfo::Create<T>();
        static constexpr bool IsComponent = eastl::is_same_v<RawType, T>;
        static constexpr bool IsTag = std::is_empty_v<T>;
        static constexpr bool IsTrackable = ComponentInfo.isTrackable;
    };

    template <typename T>
    using GetComponentType = typename ComponentType<T>::Type;

    template <typename T>
    static constexpr ComponentId GetComponentId = ComponentTraits<T>::Id;

    template <typename T>
    static constexpr ComponentInfo GetComponentInfo = ComponentTraits<T>::ComponentInfo;

    template <typename T>
    static constexpr bool IsComponent = ComponentTraits<T>::IsComponent;

    template <typename T>
    static constexpr bool IsTag = ComponentTraits<T>::IsTag;

    template <typename T>
    static constexpr bool IsTrackable = ComponentTraits<T>::IsTrackable;
}