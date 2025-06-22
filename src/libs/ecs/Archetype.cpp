#include "Archetype.hpp"

#include "ecs/EntityStorage.hpp"
#include "ecs/World.hpp"

namespace RR::Ecs
{
    ArchetypeEntityIndex Archetype::Insert(EntityId entityId)
    {
        auto index = componentsData.Insert();
        GetEntityIdData(index) = entityId;

        return index;
    }

    ArchetypeEntityIndex Archetype::Mutate(EntityStorage& entityStorage, Archetype& from, ArchetypeEntityIndex fromIndex)
    {
        ASSERT(&from != this);

        auto index = componentsData.Insert();

        for (size_t componentIndex = 0; componentIndex < componentsData.componentsInfo.size(); componentIndex++)
        {
            const auto& componentInfo = componentsData.componentsInfo[componentIndex];

           if (componentInfo.size == 0)
                continue;

           // TODO Could be faster find if we start from previous finded, to not iterate over same components id.
           auto fromComponentIndex = from.GetComponentIndex(componentInfo.id);

           if (!fromComponentIndex)
               continue;

           ComponentData dst = componentsData.GetComponentData(ArchetypeComponentIndex(componentIndex), index);
           ComponentData src = from.componentsData.GetComponentData(fromComponentIndex, fromIndex);
           componentInfo.move(dst.data, src.data);
           if (componentInfo.isTrackable)
               componentInfo.move(dst.trackedData, src.trackedData);
        }

        entityStorage.Mutate(from.GetEntityIdData(fromIndex), *this, index);
        from.Delete(entityStorage, fromIndex, false);

        return index;
    }

    void Archetype::Delete(EntityStorage& entityStorage, ArchetypeEntityIndex index, bool updateEntityRecord)
    {
        const auto lastIndex = componentsData.GetLastIndex();

        if (index != lastIndex)
            entityStorage.Move(GetEntityIdData(lastIndex), index);

        if (updateEntityRecord)
            entityStorage.Destroy(GetEntityIdData(index));

        for (size_t componentIndex = 0; componentIndex < componentsData.componentsInfo.size(); componentIndex++)
        {
            const auto& componentInfo = componentsData.componentsInfo[componentIndex];
            if (componentInfo.size == 0)
                continue;

            ComponentData removedData = componentsData.GetComponentData(ArchetypeComponentIndex(componentIndex), index);
            ComponentData lastIndexData = componentsData.GetComponentData(ArchetypeComponentIndex(componentIndex), lastIndex);

            if (index != lastIndex)
            {
                componentInfo.move(removedData.data, lastIndexData.data);
                if (componentInfo.isTrackable)
                    componentInfo.move(removedData.trackedData, lastIndexData.trackedData);
            }

            if (componentInfo.destructor != nullptr)
            {
                componentInfo.destructor(lastIndexData.data);
                if (componentInfo.isTrackable)
                    componentInfo.destructor(lastIndexData.trackedData);
            }
        }

        componentsData.entitiesCount--;
    }

    void Archetype::UpdateTrackedCache(SystemId systemId, SortedComponentsView components)
    {
        if (componentsData.trackedComponents.empty())
            return;

        if(changedComponentsMasks.empty())
            changedComponentsMasks.resize(componentsData.chunkCapacity);

        const auto componentsCount = static_cast<uint8_t>(componentsData.componentsInfo.size());
        uint64_t trackedComponentsMask = 0;

        for (auto& trackedComponent : components)
        {
            const auto componentIndex = GetComponentIndex(trackedComponent);
            const uint8_t trackedColumnIndex = componentsData.columns[componentIndex.GetRaw()].trackedColumnIndex;
            const uint8_t trackedComponentIndex = trackedColumnIndex - componentsCount;

            trackedComponentsMask |= 1ULL << static_cast<uint64_t>(trackedComponentIndex);
        }

        trackedSystems.emplace_back(systemId, trackedComponentsMask);
    }

    void Archetype::ProcessTrackedChanges(World& world)
    {
        if (componentsData.trackedComponents.empty())
            return;

        // TODO it's could be optimized by adding dirty mask for chunks/columns.

        const uint8_t componentsCount = static_cast<uint8_t>(componentsData.componentsInfo.size());
        const size_t chunksCount = componentsData.chunks.size();
        const size_t chunkCapacity = componentsData.chunkCapacity;
        const size_t entitiesCount = componentsData.entitiesCount;

        for (size_t chunkIndex = 0, entityOffset = 0; chunkIndex < chunksCount; chunkIndex++, entityOffset += chunkCapacity)
        {
            uint64_t changedChunkComponentsMask = 0;
            const size_t entitiesInChunk = eastl::min(entitiesCount - entityOffset, chunkCapacity);
            for (size_t indexInChunk = 0; indexInChunk < entitiesInChunk; indexInChunk++)
            {
                uint64_t changedComponentsMask = 0;
                for (auto& trackedComponent : componentsData.trackedComponents)
                {
                    auto& componentInfo = componentsData.componentsInfo[trackedComponent.columnIndex];
                    auto& column = componentsData.columns[trackedComponent.columnIndex];
                    auto& trackedColumn = componentsData.columns[trackedComponent.trackedColumnIndex];

                    const size_t offset = indexInChunk * column.size;
                    if (!componentInfo.compareAndAssign(trackedColumn.chunks[chunkIndex] + offset, column.chunks[chunkIndex] + offset))
                        changedComponentsMask |= 1ULL << static_cast<uint64_t>(trackedComponent.trackedColumnIndex - componentsCount);
                }

                changedChunkComponentsMask |= changedComponentsMask;
                changedComponentsMasks[indexInChunk] = changedComponentsMask;
            }

            if(changedChunkComponentsMask == 0)
                continue;

            for (const auto [systemId, mask] : trackedSystems)
            {
                if ((changedChunkComponentsMask & mask) == 0)
                    continue;

                for (size_t indexInChunk = 0; indexInChunk < changedComponentsMasks.size(); indexInChunk++)
                {
                    uint64_t changedComponentsMask = changedComponentsMasks[indexInChunk];
                    if ((mask & changedComponentsMask) == 0)
                        continue;

                    EntityId entityId = GetEntityIdData(ArchetypeEntityIndex(chunkIndex));
                    world.dispatchEventImmediately(entityId, systemId, OnChange {});
                }
            }
        }
    }
}