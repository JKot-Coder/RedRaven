#pragma once

#include "ecs/ComponentTraits.hpp"
#include "ecs/ForwardDeclarations.hpp"
#include "ecs/Hash.hpp"
#include "ecs/Index.hpp"
#include "ecs/TypeTraits.hpp"
#include <EASTL/fixed_vector.h>
#include <EASTL/vector_set.h>
#include "absl/container/flat_hash_map.h"
#include <cstddef>

namespace RR::Ecs
{
    using ArchetypeId = Index<struct ArchetypeIdTag, HashType>;
    using ArchetypeIndex = Index<struct ArchetypeIndexTag>;
    using ArchetypeComponentIndex = Index<struct ArchetypeComponentIndexTag>;

    struct ArchetypeEntityIndex final
    {
        static const uint32_t IndexBits = 18;
        static const uint32_t ChunkBits = 14;
        static_assert(IndexBits + ChunkBits == sizeof(uint32_t) * CHAR_BIT);

        static const uint32_t MaxEntitiesCount = 1U << IndexBits;
        static const uint32_t MaxChunksCount = 1U << ChunkBits;
        static const uint32_t ChunkMask = MaxChunksCount - 1;
        static const uint32_t InvalidRawIndex = 0xFFFFFFFF;

        constexpr ArchetypeEntityIndex() : rawIndex(InvalidRawIndex) { };
        constexpr ArchetypeEntityIndex(uint32_t index) : rawIndex(index) { }
        static constexpr ArchetypeEntityIndex Invalid() { return ArchetypeEntityIndex {}; };
        ArchetypeEntityIndex(uint32_t indexInChunk, uint32_t chunkIndex)
        {
            ASSERT(indexInChunk < MaxEntitiesCount);
            ASSERT(chunkIndex < MaxChunksCount);
            fields.indexInChunk = indexInChunk;
            fields.chunk = chunkIndex;
        }

        uint32_t GetChunkIndex() const { return fields.chunk; }
        uint32_t GetIndexInChunk() const { return fields.indexInChunk; }
        uint32_t GetRaw() const { return rawIndex; }

        explicit operator bool() const { return rawIndex != InvalidRawIndex; }
        bool operator==(const ArchetypeEntityIndex other) const { return rawIndex == other.rawIndex; }
        bool operator!=(const ArchetypeEntityIndex other) const { return rawIndex != other.rawIndex; }

    private:
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
    };

    template <typename... Components>
    struct ArchetypeInfo final
    {
    private:
        static constexpr HashType getArchetypeHash()
        {
            if constexpr (eastl::is_same_v<HashType, uint64_t>)
            {
                uint64_t hash = 0xcbf29ce484222325;

                auto fnv1a = [&](uint64_t value) {
                    const uint64_t prime = 0x100000001b3;
                    hash = hash ^ value;
                    hash *= prime;
                };

                (fnv1a(GetTypeHash<Components>), ...);
                return hash;
            }
            else
            {
                uint32_t hash = 0x811c9dc5;

                auto fnv1a = [&](uint64_t value) {
                    const uint32_t prime = 0x1000193;
                    hash = hash ^ value;
                    hash *= prime;
                };

                (fnv1a(GetTypeHash<Components>), ...);
                return hash;
            }
        }

    public:
        static constexpr ArchetypeId Id = ArchetypeId::FromValue(getArchetypeHash());
    };

    static constexpr ArchetypeId GetArchetypeIdForComponents(SortedComponentsView components)
    {
        auto fnv1a = [&](SortedComponentsView components) {
            if constexpr (eastl::is_same_v<HashType, uint64_t>)
            {
                uint64_t hash = 0xcbf29ce484222325;
                for (auto it = components.begin(); it != components.end(); ++it)
                {
                    const uint64_t prime = 0x100000001b3;
                    hash = hash ^ (*it).GetRaw();
                    hash *= prime;
                }
                return hash;
            }
            else
            {
                uint32_t hash = 0x811c9dc5;

                for (auto it = components.begin(); it != components.end(); ++it)
                {
                    const uint32_t prime = 0x1000193;
                    hash = hash ^ (*it).GetRaw();
                    hash *= prime;
                }
                return hash;
            }
        };

        return ArchetypeId::FromValue(fnv1a(components));
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
                componentChunks.resize(componentsInfo.size());

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
                for (size_t componentIndex = 0; componentIndex < componentsInfo.size(); componentIndex++)
                {
                    const auto& componentInfo = componentsInfo[componentIndex];
                    if (!componentInfo.destructor)
                        continue;

                    size_t entityIndex = 0;
                    for (size_t chunkIndex = 0; chunkIndex < chunks.size(); chunkIndex++)
                    {
                        for (size_t index = 0; index < chunkCapacity && entityIndex < entitiesCount; index++, entityIndex++)
                            componentInfo.destructor(componentChunks[componentIndex][chunkIndex] + index * componentsOffsetSize[componentIndex].second); //(chunk.componentChunks[i] + index * componentsOffsetSize[i].second);
                    }
                }

                for (const auto chunk : chunks)
                    delete[] chunk;
            }

            ArchetypeComponentIndex GetComponentIndex(ComponentId componentId) const
            {
                auto it = components.find(componentId);
                if (it == components.end())
                    return {};

                return ArchetypeComponentIndex(eastl::distance(components.begin(), it));
            }

            std::byte* GetComponentChunkData(ArchetypeComponentIndex componentIndex, size_t chunkIndex) const
            {
                ASSERT(componentIndex);
                ASSERT(chunkIndex < chunks.size());
                return componentChunks[componentIndex.GetRaw()][chunkIndex];
            }

            std::byte* const* GetComponentsData(ArchetypeComponentIndex componentIndex) const
            {
                ASSERT(componentIndex);
                return componentChunks[componentIndex.GetRaw()].data();
            }

            std::byte* GetComponentData(ArchetypeComponentIndex componentIndex, ArchetypeEntityIndex index) const
            {
                ASSERT(componentIndex);
                ASSERT(entitiesCount);

                const auto indexInChunk = index.GetIndexInChunk();
                const auto chunk = index.GetChunkIndex();

                ASSERT((chunk + 1 < chunks.size()) || (indexInChunk <= (entitiesCount - 1) % chunkCapacity));
                return GetComponentChunkData(componentIndex, chunk) + indexInChunk * componentsInfo[componentIndex.GetRaw()].size;
            }

            ArchetypeEntityIndex Insert()
            {
                if (entitiesCount == totalCapacity)
                {
                    totalCapacity += chunkCapacity;
                    auto chunk = chunks.emplace_back(new std::byte[chunkSize]);

                    for (size_t i = 0; i < componentsInfo.size(); i++)
                        componentChunks[i].emplace_back(chunk + componentsOffsetSize[i].first);
                }

                entitiesCount++;
                return GetLastIndex();
            }

            ArchetypeEntityIndex GetLastIndex() const
            {
                ASSERT(entitiesCount);
                ASSERT(!chunks.empty());
                return ArchetypeEntityIndex((entitiesCount - 1) % chunkCapacity, (entitiesCount - 1) / chunkCapacity);
            }

        private:
            friend struct Archetype;

            size_t chunkSize; // In bytes
            size_t chunkCapacity; // In entities
            size_t totalCapacity = 0; // In entities
            size_t entitiesCount = 0;
            size_t entitySize = 0;

            ComponentsSet components;
            eastl::fixed_vector<ComponentInfo, 32> componentsInfo;
            eastl::fixed_vector<eastl::pair<size_t, size_t>, 32> componentsOffsetSize;
            eastl::vector<std::byte*> chunks;
            eastl::fixed_vector<eastl::vector<std::byte*>, 32> componentChunks;
        };

    private:
        friend struct World;

        const auto& components() const { return componentsData.components; }

    public:
        // Components info should be sorted
        template <typename Interator>
        Archetype(Interator compInfoBegin, Interator compInfoEnd)
            : componentsData(compInfoBegin, compInfoEnd)
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

        EntityId& GetEntityIdData(ArchetypeEntityIndex index) const
        {
            return *(EntityId*)componentsData.GetComponentData(ArchetypeComponentIndex(0), index);
        }

        std::byte* GetComponentChunkData(ArchetypeComponentIndex componentIndex, size_t chunkIndex) const
        {
            ASSERT(componentIndex);
            return componentsData.GetComponentChunkData(componentIndex, chunkIndex);
        }

        std::byte* GetComponentData(ArchetypeComponentIndex componentIndex, ArchetypeEntityIndex index) const
        {
            ASSERT(componentIndex);
            return componentsData.GetComponentData(componentIndex, index);
        }

        std::byte* const* GetComponentsData(ArchetypeComponentIndex componentIndex) const
        {
            ASSERT(componentIndex);
            return componentsData.GetComponentsData(componentIndex);
        }

        template <typename Component>
        ArchetypeComponentIndex GetComponentIndex() const
        {
            if constexpr (eastl::is_same_v<Component, EntityId>)
                return ArchetypeComponentIndex(0);

            return componentsData.GetComponentIndex(GetComponentId<Component>);
        }

        template <typename ComponentId>
        ArchetypeComponentIndex GetComponentIndex(ComponentId componentId) const
        {
            return componentsData.GetComponentIndex(componentId);
        }

        const ComponentInfo& GetComponentInfo(ArchetypeComponentIndex index) const
        {
            ASSERT(index);
            return componentsData.componentsInfo[index.GetRaw()];
        }

        ArchetypeEntityIndex Insert(EntityStorage& entityStorage);
        ArchetypeEntityIndex Mutate(EntityStorage& entityStorage, Archetype& from, ArchetypeEntityIndex fromIndex);
        void Delete(EntityStorage& entityStorage, ArchetypeEntityIndex index, bool updateEntityRecord = true);

        SortedComponentsView GetComponentsView() const { return SortedComponentsView(components()); }
        size_t GetEntitiesCount() const { return componentsData.entitiesCount; }
        size_t GetChunksCount() const { return componentsData.chunks.size(); }
        size_t GetChunkCapacity() const { return componentsData.chunkCapacity; }
        size_t GetChunkSize() const { return componentsData.chunkSize; }
        size_t GetEntitySize() const { return componentsData.entitySize; }

    private:
        ComponentsData componentsData;
        absl::flat_hash_map<EventId, eastl::fixed_vector<SystemId, 8>> cache;
    };
}