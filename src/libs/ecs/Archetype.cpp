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

        for (uint8_t componentIndex = 0; componentIndex < componentsData.componentsInfo.size(); componentIndex++)
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

        for (uint8_t componentIndex = 0; componentIndex < componentsData.componentsInfo.size(); componentIndex++)
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

        OnChange event {};
        const uint8_t componentsCount = static_cast<uint8_t>(componentsData.componentsInfo.size());
        const size_t chunksCount = componentsData.chunks.size();
        const size_t chunkCapacity = componentsData.chunkCapacity;
        const size_t entitiesCount = componentsData.entitiesCount;

        static thread_local eastl::vector<uint64_t> changedComponentsMasks;
        changedComponentsMasks.reserve(chunkCapacity);

        // TODO it's could be optimized by adding dirty mask for chunks/columns.
        for (uint32_t chunkIndex = 0, entityOffset = 0; chunkIndex < chunksCount; chunkIndex++, entityOffset += uint32_t(chunkCapacity))
        {
            uint64_t changedChunkComponentsMask = 0;
            const size_t entitiesInChunk = eastl::min(entitiesCount - entityOffset, chunkCapacity);

            if(entitiesInChunk == 0)
                break;

            eastl::fill_n(changedComponentsMasks.data(), entitiesInChunk, 0);

            for (auto& trackedComponent : componentsData.trackedComponents)
            {
                auto& componentInfo = componentsData.componentsInfo[trackedComponent.columnIndex];
                auto& column = componentsData.columns[trackedComponent.columnIndex];
                auto& trackedColumn = componentsData.columns[trackedComponent.trackedColumnIndex];

                for (size_t indexInChunk = 0; indexInChunk < entitiesInChunk; indexInChunk++)
                {
                    const size_t offset = indexInChunk * column.size;
                    if (!componentInfo.compareAndAssign(trackedColumn.chunks[chunkIndex] + offset, column.chunks[chunkIndex] + offset))
                    {
                        uint64_t mask = 1ULL << static_cast<uint64_t>(trackedComponent.trackedColumnIndex - componentsCount);
                        *(changedComponentsMasks.data() + indexInChunk) |= mask;
                        changedChunkComponentsMask |= mask;
                    }
                }
            }

            if(changedChunkComponentsMask == 0)
                continue;

            for (const auto [systemId, mask] : trackedSystems)
            {
                if ((changedChunkComponentsMask & mask) == 0)
                    continue;

                const ArchetypeEntityIndex begin = ArchetypeEntityIndex(0, chunkIndex);
                ArchetypeEntitySpan span(*this, begin, begin);

                for (size_t indexInChunk = 0; indexInChunk < entitiesInChunk; indexInChunk++)
                {
                    uint64_t changedComponentsMask = *(changedComponentsMasks.data() + indexInChunk);
                    if ((mask & changedComponentsMask) == 0)
                    {
                        span.begin = span.end;
                        span.end = inc(span.end);
                        world.dispatchEventImmediately(span, systemId, event);
                        continue;
                    }

                    span.end = inc(span.end);
                }
                if(span.begin != span.end)
                    world.dispatchEventImmediately(span, systemId, event);
            }
        }
    }
}