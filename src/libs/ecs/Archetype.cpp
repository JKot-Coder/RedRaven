#include "Archetype.hpp"

#include "ecs/EntityStorage.hpp"

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
               componentInfo.copy(dst.trackedData, src.trackedData);
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
}