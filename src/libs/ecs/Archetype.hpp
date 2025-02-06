#pragma once

#include "ecs/ComponentTraits.hpp"
#include "ecs/Hash.hpp"
#include "ecs/Index.hpp"
#include "ecs/TypeTraits.hpp"
#include "ska/flat_hash_map.h"
#include <numeric>

namespace RR::Ecs
{
    using ArchetypeId = Index<struct ArchetypeIdTag, HashType>;
    using ArchetypeEntityIndex = Index<struct ArchetypeEntityIndexTag>;

    class ComponentInfoView
    {
    public:
        using iterator = const ComponentInfo*;

        ComponentInfoView(const iterator& begin, const iterator& end) : begin_(begin), end_(end) { }

        iterator begin() const { return begin_; }
        iterator end() const { return end_; }

    private:
        iterator begin_;
        iterator end_;
    };


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

            (hash_64_fnv1a(GetTypeHash<Components>()), ...);
            return hash;
        }

    public:
        static constexpr ArchetypeId Id = ArchetypeId::FromValue(getArchetypeHash());
    };

    class Archetype
    {
    public:
        struct ComponentData
        {
            using Allocator = EASTLAllocatorType;

            ComponentData(size_t chunkSizePower, size_t sizeOfElement, size_t alignmentOfElement, const Allocator& allocator)
                : chunkSize(1 << chunkSizePower),
                  sizeOfElement(sizeOfElement),
                  alignmentOfElement(alignmentOfElement),
                  containerAlignment(std::lcm(chunkSize, alignmentOfElement)),
                  allocator(allocator)
            {
            }

            ~ComponentData()
            {
                for (char* data : chunks)
                    allocator.deallocate(data, chunkSize * sizeOfElement);
            }

            void AllocateChunk()
            {
                capacity += chunkSize;
                chunks.push_back((char*)allocator.allocate(chunkSize * sizeOfElement, containerAlignment, 0));
            }
/*
            char* GetData(size_t linearIndex)
            {
                return GetData(linearIndex >> chunkSizePower, linear_index & chunkMask);
            }
*/
            char* Get(size_t chunk, size_t index) const
            {
                ASSERT(chunk < chunks.size());
                ASSERT(index < chunkSize);
                return chunks[chunk] + (index) * sizeOfElement;
            }

            eastl::vector<char*> chunks;
            // size_t size;
            size_t chunkSize = 0;
            size_t sizeOfElement = 0;
            size_t alignmentOfElement = 0;
            size_t containerAlignment = 0;
            size_t capacity = 0;
            Allocator allocator;
        };

    public:
        Archetype(size_t chunkSizePower, ComponentInfoView components)
            : chunkSize(1 << chunkSizePower)
        {
            for (ComponentInfo componentInfo : components)
            {
                componentsData.emplace_back(chunkSizePower, componentInfo.size, componentInfo.alignment, ComponentData::Allocator {});
                componentIdToDataIndex[componentInfo.id] = componentsData.size() - 1;
            }
        }

        ArchetypeEntityIndex Insert()
        {
            expand(1);
            return ArchetypeEntityIndex(entityCount++);
        }

        const ComponentData* GetComponentData(ComponentId componentId) const
        {
            auto it = componentIdToDataIndex.find(componentId);
            return it != componentIdToDataIndex.end() ? &componentsData[it->second] : nullptr;
        };

        size_t GetEntityCount() const { return entityCount; }
        size_t GetChunkCount() const { return chunkCount; }
        size_t GetChunkSize() const { return chunkSize; }

    private:
        void expand(size_t requiredEntityCount)
        {
            while (entityCount + requiredEntityCount > capacity)
            {
                capacity += chunkSize;
                chunkCount++;
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
        size_t chunkCount = 0;
        eastl::vector<ComponentData> componentsData;
        ska::flat_hash_map<ComponentId, size_t> componentIdToDataIndex;
    };
}