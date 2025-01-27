#pragma once

#include "ecs/Hash.hpp"
#include "ecs/Index.hpp"
#include "ecs/TypeTraits.hpp"
#include "ska/flat_hash_map.h"

namespace RR::Ecs
{
    struct ComponentId : public Index<ComponentId, HashType>{};
    struct ArchetypeId : public Index<ArchetypeId, HashType>{};
    struct ArchetypeEntityIndex : public Index<ArchetypeEntityIndex>{};
    
    class ComponentsView
    {
    public:
        using iterator = const ComponentId*;

        ComponentsView(const iterator& begin, const iterator& end) : begin_(begin), end_(end) {}

        iterator begin() const { return begin_; }
        iterator end() const { return end_; }

    private:
        iterator begin_;
        iterator end_;
    };

    namespace detail
    {
        template <typename ComponentArg>
        ComponentId GetComponentId()
        {
            using ComponentArg = typename eastl::remove_reference<ComponentArg>::type;
            using Component = typename ArgType::Component;

            return ComponentId(TypeTraits<Component>::Id.Value());
        }
    }

    template <typename... Components>
    struct ArchetypeInfo
    {
    private:
        static constexpr HashType getArchetypeHash()
        {
            static_assert(eastl::is_same_v<HashType, uint64_t>, "Update hash combine function");

            uint64_t hash = 0xcbf29ce484222325;

            auto hash_64_fnv1a = [&](uint64_t value) {
                uint64_t prime = 0x100000001b3;
                hash = hash ^ value;
                hash *= prime;
            };

            (hash_64_fnv1a(TypeTraits<Components>::Hash), ...);
            return hash;
        }

    public:
        static constexpr ArchetypeId Id = ArchetypeId::FromValue(getArchetypeHash());
    };

    template <typename Allocator = EASTLAllocatorType>
    class Archetype
    {
    private:
        struct ComponentData
        {
            ComponentData(size_t chunkSizePower, size_t sizeOfElement, const Allocator& allocator) : chunkSize(1 << chunkSizePower), sizeOfElement(sizeOfElement), allocator(allocator) { }

            ~ComponentData()
            {
                for (char* data : chunks)
                    allocator.deallocate(data, chunkSize * sizeOfElement);
            }

            void AllocateChunk()
            {
                capacity += chunkSize;
                chunks.push_back(allocator.allocate(chunkSize * sizeOfElement, containerAlignment, 0));
            }

            eastl::vector<char*, Allocator> chunks;
            // size_t size;
            size_t chunkSize = 0;
            // size_t  containerAlignment;
            size_t capacity = 0;
            size_t sizeOfElement = 0;
            Allocator allocator;
        };

    public:
        Archetype(size_t chunkSizePower, ComponentsView componets) : Archetype(chunkSizePower, eastl::move(componets), Allocator()) { };
        Archetype(size_t chunkSizePower, ComponentsView componets, const Allocator& allocator)
            : chunkSize(1 << chunkSizePower)
        {
            for (ComponentId componentId : componets)
            {
                componentsData.push_back(ComponentData(chunkSizePower, allocator));
                componentIdToDataIndex[componentId] = componentsData.size() - 1;
            }
        }

        ArchetypeEntityIndex Insert()
        {
            expand(1);
            return ArchetypeEntityIndex(entityCount++);
        }

    private:
        void expand(size_t requiredEntityCount)
        {
            while (entityCount + requiredEntityCount > capacity)
            {
                capacity += chunkSize;
                for (ComponentData& componentData : componentsData)
                {
                    componentData.AllocateChunk();
                    ASSERT(capacity == componentData.capacity);
                }
            }
        }

        size_t entityCount = 0;
        size_t capacity = 0;
        size_t chunkSize = 0;
        eastl::vector<ComponentData> componentsData;
        ska::flat_hash_map<ComponentId, size_t> componentIdToDataIndex;
    };
}

namespace eastl
{
    using namespace RR::Ecs;
    template<>
    struct hash<ArchetypeId> : eastl::hash<RR::Ecs::Index<ArchetypeId, size_t>> {};
}
