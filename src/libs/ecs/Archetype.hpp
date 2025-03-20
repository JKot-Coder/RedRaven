#pragma once

#include "ecs/ComponentTraits.hpp"
#include "ecs/ForwardDeclarations.hpp"
#include "ecs/Hash.hpp"
#include "ecs/Index.hpp"
#include "ecs/TypeTraits.hpp"
#include "ska/flat_hash_map.h"
#include <EASTL/fixed_vector.h>
#include <EASTL/sort.h>
#include <EASTL/vector_set.h>

namespace RR::Ecs
{
    using ArchetypeId = Index<struct ArchetypeIdTag, HashType>;
    using ArchetypeComponentIndex = Index<struct ArchetypeComponentIndexTag>;

    struct ArchetypeEntityIndex final
    {
        static const uint32_t IndexBits = 24;
        static const uint32_t ChunkBits = 8;
        static_assert(IndexBits + ChunkBits == sizeof(uint32_t) * 8);

        static const uint32_t MaxEntitiesCount = 1 << IndexBits;
        static const uint32_t MaxChunksCount = 1 << ChunkBits;
        static const uint32_t ChunkMask = MaxChunksCount - 1;
        static const uint32_t InvalidRawIndex = 0xFFFFFFFF;

        struct IndexFields
        {
            uint32_t indexInChunk : IndexBits;
            uint32_t chunk : ChunkBits;
        };
        union
        {
            IndexFields fields;
            uint32_t rawIndex;
        };

        constexpr ArchetypeEntityIndex() : rawIndex(InvalidRawIndex) { };
        constexpr ArchetypeEntityIndex(uint32_t index) : rawIndex(index) { }
        static constexpr ArchetypeEntityIndex Invalid() { return ArchetypeEntityIndex {}; };
        ArchetypeEntityIndex(uint32_t indexInChunk, uint32_t chunkIndex)
        {
            fields.indexInChunk = indexInChunk;
            fields.chunk = chunkIndex;
        }

        uint32_t GetChunkIndex() const { return fields.chunk; }
        uint32_t GetIndexInChunk() const { return fields.indexInChunk; }
        uint32_t GetRaw() const { return rawIndex; }

        bool operator==(const ArchetypeEntityIndex other) const { return rawIndex == other.rawIndex; }
        bool operator!=(const ArchetypeEntityIndex other) const { return rawIndex != other.rawIndex; }
    };

    template <typename... Components>
    struct ArchetypeInfo final
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

            (hash_64_fnv1a(GetTypeHash<Components>), ...);
            return hash;
        }

    public:
        static constexpr ArchetypeId Id = ArchetypeId::FromValue(getArchetypeHash());
    };

    static constexpr ArchetypeId GetArchetypeIdForComponents(SortedComponentsView components)
    {
        static_assert(eastl::is_same_v<HashType, uint64_t>, "Update hash combine function");

        uint64_t hash = 0xcbf29ce484222325;

        auto hash_64_fnv1a = [&](uint64_t value) {
            uint64_t prime = 0x100000001b3;
            hash = hash ^ value;
            hash *= prime;
        };

        for (auto it = components.begin(); it != components.end(); ++it)
            hash_64_fnv1a((*it).GetRaw());
        return ArchetypeId::FromValue(hash);
    }

    struct Archetype final
    {
    public:
        class ComponentsData
        {
            static constexpr size_t BaseChunkSize = 16 * 1024; // 16 kb
            static constexpr size_t BaseChunkEntityCount = 100;

        public:
            template <typename Iterator>
            ComponentsData(Iterator compInfoBegin, Iterator compInfoEnd)
            {
                ASSERT(IsPowerOfTwo(BaseChunkSize));
                ASSERT(BaseChunkSize > 0);

                size_t entitySizeBytes = 0;
                for (Iterator it = compInfoBegin; it != compInfoEnd; ++it)
                {
                    ComponentInfo& componentInfo = *it;
                    componentsInfo.push_back(componentInfo);
                    components.push_back_unsorted(componentInfo.id);
                    entitySizeBytes += componentInfo.size;
                }
                ASSERT(componentsInfo[0].id == GetComponentId<EntityId>);

                size_t chunkSizeBytes = entitySizeBytes * BaseChunkEntityCount;
                chunkSizeBytes = ((chunkSizeBytes / BaseChunkSize) + 1) * BaseChunkSize;
                chunkCapacity = chunkSizeBytes / entitySizeBytes;

                componentsOffsetSize.resize(componentsInfo.size());

                for (;;)
                {
                    size_t offset = 0;
                    size_t index = 0;

                    for (const auto& componentInfo : componentsInfo)
                    {
                        offset = AlignTo(offset, componentInfo.alignment);
                        componentsOffsetSize[index++] = {offset, componentInfo.size};
                        offset += chunkCapacity * componentInfo.size;
                    }

                    if (UNLIKELY(offset > chunkSizeBytes))
                    {
                        --chunkCapacity;
                        continue;
                    }

                    break;
                }

                entitySize = entitySizeBytes;
                chunkSize = chunkSizeBytes;
            }

            ~ComponentsData()
            {
                for (size_t i = 0; i < componentsInfo.size(); i++)
                {
                    const auto& componentInfo = componentsInfo[i];
                    if (!componentInfo.destructor)
                        continue;

                    size_t entityIndex = 0;
                    for (const auto chunk : chunks)
                    {
                        for (size_t index = 0; index < chunkCapacity && entityIndex < entitiesCount; index++, entityIndex++)
                            componentInfo.destructor(chunk + componentsOffsetSize[i].first + index * componentsOffsetSize[i].second);
                    }
                }

                for (auto chunk : chunks)
                    delete[] chunk;
            }

            ArchetypeComponentIndex GetComponentIndex(ComponentId componentId) const
            {
                auto it = components.find(componentId);
                if (it == components.end())
                    return {};

                return ArchetypeComponentIndex(eastl::distance(components.begin(), it));
            }

            std::byte* GetData(ArchetypeComponentIndex componentIndex, size_t chunkIndex) const
            {
                ASSERT(componentIndex);
                ASSERT(chunkIndex < chunks.size());

                return chunks[chunkIndex] + componentsOffsetSize[componentIndex.GetRaw()].first;
            }

            std::byte* GetData(ArchetypeComponentIndex componentIndex, ArchetypeEntityIndex index) const
            {
                ASSERT(entitiesCount);

                const auto indexInChunk = index.GetIndexInChunk();
                const auto chunk = index.GetChunkIndex();

                ASSERT((chunk + 1 < chunks.size()) || (indexInChunk <= (entitiesCount - 1) % chunkCapacity));
                return GetData(componentIndex, chunk) + indexInChunk * componentsOffsetSize[componentIndex.GetRaw()].second;
            }

            ArchetypeEntityIndex Insert()
            {
                if (entitiesCount == totalCapacity)
                {
                    totalCapacity += chunkCapacity;
                    chunks.push_back(new std::byte[chunkSize]);
                }

                entitiesCount++;
                return GetLastIndex();
            }

            ArchetypeEntityIndex GetLastIndex() const
            {
                ASSERT(entitiesCount);
                ASSERT(chunks.size() > 0);
                return ArchetypeEntityIndex((entitiesCount - 1) % chunkCapacity, chunks.size() - 1);
            }

        private:
            friend struct Archetype;

            size_t chunkSize; // In bytes
            size_t chunkCapacity; // In entities
            size_t totalCapacity = 0; // In entities
            size_t entitiesCount = 0;
            size_t entitySize = 0;

            FixedVectorSet<ComponentId, 32> components;
            eastl::fixed_vector<ComponentInfo, 32> componentsInfo;
            eastl::fixed_vector<eastl::pair<size_t, size_t>, 32> componentsOffsetSize;
            eastl::vector<std::byte*> chunks;
        };

    private:
        friend struct World;

        const auto& components() const { return componentsData.components; }

    public:
        // Components info should be sorted
        template <typename Interator>
        Archetype(ArchetypeId id, Interator compInfoBegin, Interator compInfoEnd)
            : id(id), componentsData(compInfoBegin, compInfoEnd)
        {
        }

        bool HasAll(SortedComponentsView components_) const
        {
            return std::includes(components().begin(), components().end(), components_.begin(), components_.end());
        }

        bool HasAny(SortedComponentsView components_) const
        {
            auto it1 = components().begin();
            auto it2 = components_.begin();

            while (it1 != components().end() && it2 != components_.end())
            {
                if (*it1 < *it2)
                {
                    ++it1;
                }
                else if (*it2 < *it1)
                {
                    ++it2;
                }
                else
                    return true;
            }
            return false;
        }

        std::byte* GetData(ArchetypeComponentIndex componentIndex, ArchetypeEntityIndex index) const
        {
            return componentsData.GetData(componentIndex, index);
        }

        std::byte* GetData(ArchetypeComponentIndex componentIndex, size_t chunkIndex) const
        {
            return componentsData.GetData(componentIndex, chunkIndex);
        }

        std::byte* GetData(ComponentId componentId, ArchetypeEntityIndex index) const
        {
            // Assume components present, no additional check for perf
            const auto componentIndex = componentsData.GetComponentIndex(componentId);
            return componentsData.GetData(componentIndex, index);
        }

        ArchetypeComponentIndex GetComponentIndex(ComponentId componentId) const { return componentsData.GetComponentIndex(componentId); }
        const ComponentInfo& GetComponentInfo(ArchetypeComponentIndex index) const
        {
            ASSERT(index);
            return componentsData.componentsInfo[index.GetRaw()];
        }

        ArchetypeEntityIndex Insert(EntityStorage& entityStorage, EntityId entityId);
        ArchetypeEntityIndex Mutate(EntityStorage& entityStorage, Archetype& from, ArchetypeEntityIndex fromIndex);
        void Delete(EntityStorage& entityStorage, ArchetypeEntityIndex index, bool updateEntityRecord = true);

        SortedComponentsView GetComponentsView() const { return SortedComponentsView(components()); }
        size_t GetEntitiesCount() const { return componentsData.entitiesCount; }
        size_t GetChunksCount() const { return componentsData.chunks.size(); }
        size_t GetChunkCapacity() const { return componentsData.chunkCapacity; }
        size_t GetChunkSize() const { return componentsData.chunkSize; }
        size_t GetEntitySize() const { return componentsData.entitySize; }
        ArchetypeId GetId() const { return id; }

    private:
        ArchetypeId id;
        ComponentsData componentsData;
        ska::flat_hash_map<EventId, eastl::fixed_vector<SystemId, 8>> cache;
    };
}