#pragma once

#include "absl/container/flat_hash_map.h"
#include "ecs/ArchetypeEntityIndex.hpp"
#include "ecs/meta/ComponentTraits.hpp"
#include "ecs/ForwardDeclarations.hpp"
#include "ecs/Hash.hpp"
#include "ecs/Index.hpp"
#include "ecs/meta/TypeTraits.hpp"
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

                (fnv1a(Meta::GetTypeHash<Components>), ...);
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

                (fnv1a(Meta::GetTypeHash<Components>), ...);
                return hash;
            }
        }

    public:
        static constexpr ArchetypeId Id = ArchetypeId::FromValue(getArchetypeHash());
    };

    static constexpr ArchetypeId GetArchetypeIdForComponents(Meta::SortedComponentsView components)
    {
        auto fnv1a = [&](Meta::SortedComponentsView components) {
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

                size_t entitySizeBytes = 0;
                size_t trackedComponentsCount = 0;

                for (Iterator it = compInfoBegin; it != compInfoEnd; ++it)
                {
                    const Meta::ComponentInfo& componentInfo = *it;
                    isSingleton = isSingleton || componentInfo.isSingleton;
                    componentsInfo.push_back(componentInfo);
                    components.push_back_unsorted(componentInfo.id);
                    entitySizeBytes += componentInfo.isTrackable ? componentInfo.size * 2 : componentInfo.size;
                    trackedComponentsCount += componentInfo.isTrackable ? 1 : 0;
                }
                ASSERT(componentsInfo[0].id == Meta::GetComponentId<EntityId>);

                size_t chunkSizeBytes = entitySizeBytes * BaseChunkEntityCount;
                chunkSizeBytes = ((chunkSizeBytes / BaseChunkSize) + 1) * BaseChunkSize;
                chunkCapacity = !isSingleton ? chunkSizeBytes / entitySizeBytes : 1;

                size_t realChunkSize = 0;
                columns.resize(componentsInfo.size() + trackedComponentsCount);
                trackedComponents.resize(trackedComponentsCount);
                // Todo assert total components count less than 256
                // Todo assert trackedComponentsCount less than 64
                for (;;)
                {
                    size_t offset = 0;
                    uint8_t componentIndex = 0;
                    size_t trackedComponentIndex = 0;
                    uint8_t trackedColumnIndex = uint8_t(componentsInfo.size());

                    for (const auto& componentInfo : componentsInfo)
                    {
                        auto initColumn = [this, &offset, &componentInfo, &realChunkSize](size_t componentIndex) {
                            auto& column = columns[componentIndex];

                            column.size = componentInfo.size;
                            offset = AlignTo(offset, componentInfo.alignment);
                            column.offset = offset;
                            offset += chunkCapacity * componentInfo.size;
                            realChunkSize = offset;
                        };

                        columns[componentIndex].trackedColumnIndex = componentInfo.isTrackable ? trackedColumnIndex : InvalidColumnIndex;
                        if (componentInfo.isTrackable)
                        {
                            initColumn(trackedColumnIndex);
                            trackedComponents[trackedComponentIndex].columnIndex = componentIndex;
                            trackedComponents[trackedComponentIndex].trackedColumnIndex = trackedColumnIndex;
                            trackedComponentIndex++;
                            trackedColumnIndex++;
                        }
                        initColumn(componentIndex++);
                    }

                    if (UNLIKELY(offset > chunkSizeBytes) && !isSingleton)
                    {
                        --chunkCapacity;
                        continue;
                    }

                    break;
                }

                entitySize = !isSingleton ? entitySizeBytes : realChunkSize;
                chunkSize = !isSingleton ? chunkSizeBytes : realChunkSize;
            }

            ~ComponentsData()
            {
                auto destroyComponent = [this](const Meta::ComponentInfo& componentInfo, eastl::span<std::byte*> chunks) {
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
                        destroyComponent(componentsInfo[componentIndex], columns[columns[componentIndex].trackedColumnIndex].chunks);
                }

                chunks.clear();
            }

            ArchetypeComponentIndex GetComponentIndex(Meta::ComponentId componentId) const
            {
                auto it = components.find(componentId);
                if (it == components.end())
                    return {};

                return ArchetypeComponentIndex(uint8_t(eastl::distance(components.begin(), it)));
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
                        (column.trackedColumnIndex != InvalidColumnIndex) ? columns[column.trackedColumnIndex].chunks[chunkIndex] + indexInChunk * column.size : nullptr};
            }

            ArchetypeEntityIndex Insert()
            {
                ASSERT(!isSingleton || entitiesCount == 0);

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
                return ArchetypeEntityIndex(uint32_t((entitiesCount - 1) % chunkCapacity), uint32_t((entitiesCount - 1) / chunkCapacity));
            }

            ArchetypeEntityIndex End() const
            {
                return ArchetypeEntityIndex(uint32_t(entitiesCount % chunkCapacity), uint32_t(entitiesCount / chunkCapacity));
            }

        private:
            friend struct Archetype;

            bool isSingleton = false;

            size_t chunkSize; // In bytes
            size_t chunkCapacity; // In entities
            size_t totalCapacity = 0; // In entities
            size_t entitiesCount = 0;
            size_t entitySize = 0;

            static constexpr uint8_t InvalidColumnIndex = 0xff;

            struct Column
            {
                uint8_t trackedColumnIndex;
                size_t size;
                size_t offset : 32;
                eastl::fixed_vector<std::byte*, 16> chunks;
            };

            struct TrackedComponent
            {
                uint8_t columnIndex;
                uint8_t trackedColumnIndex;
            };

            Meta::ComponentsSet components;
            eastl::fixed_vector<Meta::ComponentInfo, 32> componentsInfo;
            eastl::fixed_vector<TrackedComponent, 32> trackedComponents;
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

        bool HasAll(Meta::SortedComponentsView components_) const
        {
            return Meta::SortedComponentsView(components()).IsIncludes(components_);
        }

        bool HasAny(Meta::SortedComponentsView components_) const
        {
            return Meta::SortedComponentsView(components()).IsIntersects(components_);
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

            return componentsData.GetComponentIndex(Meta::GetComponentId<Component>);
        }

        template <typename ComponentId>
        ArchetypeComponentIndex GetComponentIndex(ComponentId componentId) const
        {
            return componentsData.GetComponentIndex(componentId);
        }

        const Meta::ComponentInfo& GetComponentInfo(ArchetypeComponentIndex index) const
        {
            ASSERT(index);
            return componentsData.componentsInfo[index.GetRaw()];
        }

        [[nodiscard]] ArchetypeEntityIndex begin() const { return ArchetypeEntityIndex(0, 0); }
        [[nodiscard]] ArchetypeEntityIndex end() const { return componentsData.End(); }
        [[nodiscard]] ArchetypeEntityIndex inc(ArchetypeEntityIndex index) const
        {
            uint32_t indexInChunk = index.GetIndexInChunk() + 1;
            uint32_t chunkIndex = index.GetChunkIndex();

            if (indexInChunk == componentsData.chunkCapacity)
            {
                indexInChunk = 0;
                ++chunkIndex;
            }

            return ArchetypeEntityIndex(indexInChunk, chunkIndex);
        }

        ArchetypeEntityIndex Insert(EntityId entityId);
        ArchetypeEntityIndex Mutate(EntityStorage& entityStorage, Archetype& from, ArchetypeEntityIndex fromIndex);
        void Delete(EntityStorage& entityStorage, ArchetypeEntityIndex index, bool updateEntityRecord = true);

        Meta::SortedComponentsView GetComponentsView() const { return Meta::SortedComponentsView(components()); }
        size_t GetEntitiesCount() const { return componentsData.entitiesCount; }
        size_t GetChunksCount() const { return componentsData.chunks.size(); }
        size_t GetChunkCapacity() const { return componentsData.chunkCapacity; }
        size_t GetChunkSize() const { return componentsData.chunkSize; }
        size_t GetEntitySize() const { return componentsData.entitySize; }

        template <typename Component, typename... Args>
        void ConstructComponent(ArchetypeEntityIndex index, ArchetypeComponentIndex componentIndex, Args&&... args)
        {
            if constexpr (Meta::IsTag<Component>)
                return;

            ComponentData componentData = componentsData.GetComponentData(componentIndex, index);

            if constexpr (std::is_aggregate_v<Component>)
            {
                new (componentData.data) Component {std::forward<Args>(args)...};
            }
            else
                new (componentData.data) Component(std::forward<Args>(args)...);

            if constexpr (Meta::IsTrackable<Component>)
            {
                if constexpr (std::is_aggregate_v<Component>)
                {
                    new (componentData.trackedData) Component {std::forward<Args>(args)...};
                }
                else
                    new (componentData.trackedData) Component(std::forward<Args>(args)...);
            }
        }

        void MoveComponentFrom(ArchetypeEntityIndex index, ArchetypeComponentIndex componentIndex, void* src)
        {
            const auto& componentInfo = GetComponentInfo(componentIndex);
            if (!componentInfo.size)
                return;

            ASSERT(src);
            ComponentData componentData = componentsData.GetComponentData(componentIndex, index);

            if (componentData.trackedData)
                componentInfo.copy(componentData.trackedData, src);
            componentInfo.move(componentData.data, src);
        }

        void UpdateTrackedCache(SystemId systemId, Meta::SortedComponentsView components);
        void ProcessTrackedChanges(World& world);

    private:
        ComponentsData componentsData;
        absl::flat_hash_map<EventId, eastl::fixed_vector<SystemId, 8>, Ecs::DummyHasher<EventId>> cache;

        struct TrackedSystem
        {
            SystemId system;
            uint64_t trackedComponentsMask;
            TrackedSystem(SystemId system, uint64_t trackedComponentsMask) : system(system), trackedComponentsMask(trackedComponentsMask) { }
        };
        eastl::fixed_vector<TrackedSystem, 8> trackedSystems;
    };
}