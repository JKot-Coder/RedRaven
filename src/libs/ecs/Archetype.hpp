#pragma once

#include "absl/container/flat_hash_map.h"
#include "ecs/ArchetypeEntityIndex.hpp"
#include "ecs/ComponentTraits.hpp"
#include "ecs/ForwardDeclarations.hpp"
#include "ecs/Hash.hpp"
#include "ecs/Index.hpp"
#include "ecs/TypeTraits.hpp"
#include <EASTL/fixed_vector.h>
#include <EASTL/vector_set.h>
#include <cstddef>

namespace RR::Ecs
{
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
        struct ComponentData
        {
            std::byte* data;
            std::byte* trackedData;
        };

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
                size_t trackedComponentsCount = 0;
                for (Iterator it = compInfoBegin; it != compInfoEnd; ++it)
                {
                    ComponentInfo& componentInfo = *it;
                    componentsInfo.push_back(componentInfo);
                    components.push_back_unsorted(componentInfo.id);
                    entitySizeBytes += componentInfo.isTrackable ? componentInfo.size * 2 : componentInfo.size;
                    trackedComponentsCount += componentInfo.isTrackable ? 1 : 0;
                }
                ASSERT(componentsInfo[0].id == GetComponentId<EntityId>);

                size_t chunkSizeBytes = entitySizeBytes * BaseChunkEntityCount;
                chunkSizeBytes = ((chunkSizeBytes / BaseChunkSize) + 1) * BaseChunkSize;
                chunkCapacity = chunkSizeBytes / entitySizeBytes;
                columns.resize(componentsInfo.size() + trackedComponentsCount);

                for (;;)
                {
                    size_t offset = 0;
                    size_t componentIndex = 0;
                    size_t trackedComponentIndex = componentsInfo.size();

                    for (const auto& componentInfo : componentsInfo)
                    {
                        auto initColumn = [this, &offset, &componentInfo](size_t componentIndex) {
                            auto& column = columns[componentIndex];

                            column.size = componentInfo.size;
                            offset = AlignTo(offset, componentInfo.alignment);
                            column.offset = offset;
                            offset += chunkCapacity * componentInfo.size;
                        };

                        initColumn(componentIndex++);
                        if (componentInfo.isTrackable)
                            initColumn(trackedComponentIndex++);
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
                auto destroyComponent = [this](const ComponentInfo& componentInfo, eastl::span<std::byte*> chunks) {
                    if (!componentInfo.destructor)
                        return;

                    size_t entityIndex = 0;
                    for (auto* chunk : chunks)
                        for (size_t index = 0; index < chunkCapacity && entityIndex < entitiesCount; index++, entityIndex++)
                            componentInfo.destructor(chunk + index * componentInfo.size);
                };

                for (size_t componentIndex = 0; componentIndex < componentsInfo.size(); componentIndex++)
                {
                    destroyComponent(componentsInfo[componentIndex], columns[componentIndex].chunks);

                    if (componentsInfo[componentIndex].isTrackable)
                        destroyComponent(componentsInfo[componentIndex], columns[columns[componentIndex].trackedComponentIndex.GetRaw()].chunks);
                }

                chunks.clear();
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
                return columns[componentIndex.GetRaw()].chunks[chunkIndex];
            }

            std::byte* const* GetComponentsData(ArchetypeComponentIndex componentIndex) const
            {
                ASSERT(componentIndex);
                return columns[componentIndex.GetRaw()].chunks.data();
            }

            ComponentData GetComponentData(ArchetypeComponentIndex componentIndex, ArchetypeEntityIndex index) const
            {
                ASSERT(componentIndex);
                ASSERT(entitiesCount);
                ASSERT(index);

                const auto indexInChunk = index.GetIndexInChunk();
                const auto chunkIndex = index.GetChunkIndex();
                const auto& column = columns[componentIndex.GetRaw()];

                ASSERT((chunkIndex + 1 < chunks.size()) || (indexInChunk <= (entitiesCount - 1) % chunkCapacity));
                return {column.chunks[chunkIndex] + indexInChunk * column.size,
                        column.trackedComponentIndex.IsValid() ? columns[column.trackedComponentIndex.GetRaw()].chunks[chunkIndex] + indexInChunk * column.size : nullptr};
            }

            ArchetypeEntityIndex Insert()
            {
                if (entitiesCount == totalCapacity)
                {
                    totalCapacity += chunkCapacity;
                    std::byte* chunk = chunks.emplace_back(new std::byte[chunkSize]).get();

                    for (auto& column : columns)
                        column.chunks.emplace_back(chunk + column.offset);
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

            struct Column
            {
                ArchetypeComponentIndex trackedComponentIndex;
                size_t size;
                size_t offset : 32;
                eastl::fixed_vector<std::byte*, 16> chunks;
            };

            ComponentsSet components;
            eastl::fixed_vector<ComponentInfo, 32> componentsInfo;
            eastl::fixed_vector<Column, 32> columns;
            eastl::vector<eastl::unique_ptr<std::byte[]>> chunks;
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
            return SortedComponentsView(components()).IsIncludes(components_);
        }

        bool HasAny(SortedComponentsView components_) const
        {
            return SortedComponentsView(components()).IsIntersects(components_);
        }

        EntityId& GetEntityIdData(ArchetypeEntityIndex index) const
        {
            return *(EntityId*)componentsData.GetComponentData(ArchetypeComponentIndex(0), index).data;
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

        ArchetypeEntityIndex Insert(EntityId entityId);
        ArchetypeEntityIndex Mutate(EntityStorage& entityStorage, Archetype& from, ArchetypeEntityIndex fromIndex);
        void Delete(EntityStorage& entityStorage, ArchetypeEntityIndex index, bool updateEntityRecord = true);

        SortedComponentsView GetComponentsView() const { return SortedComponentsView(components()); }
        size_t GetEntitiesCount() const { return componentsData.entitiesCount; }
        size_t GetChunksCount() const { return componentsData.chunks.size(); }
        size_t GetChunkCapacity() const { return componentsData.chunkCapacity; }
        size_t GetChunkSize() const { return componentsData.chunkSize; }
        size_t GetEntitySize() const { return componentsData.entitySize; }

        template <typename Component, typename... Args>
        void ConstructComponent(ArchetypeEntityIndex index, ArchetypeComponentIndex componentIndex, Args&&... args)
        {
            if constexpr (IsTag<Component>)
                return;

            ComponentData componentData = componentsData.GetComponentData(componentIndex, index);
            new (componentData.data) Component { std::forward<Args>(args)... };

            if constexpr (IsTrackable<Component>)
                new (componentData.trackedData) Component { std::forward<Args>(args)... };
        }

        void MoveComponentFrom(ArchetypeEntityIndex index, ArchetypeComponentIndex componentIndex, void* src)
        {
            ComponentData componentData = componentsData.GetComponentData(componentIndex, index);
            const auto& componentInfo = GetComponentInfo(componentIndex);
            if (componentInfo.size)
            {
                if(componentData.trackedData)
                    componentInfo.copy(componentData.trackedData, src);
                componentInfo.move(componentData.data, src);
            }
        }

    private:
        ComponentsData componentsData;
        absl::flat_hash_map<EventId, eastl::fixed_vector<SystemId, 8>> cache;
    };
}