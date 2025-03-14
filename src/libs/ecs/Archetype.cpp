#include "Archetype.hpp"

#include "ecs/EntityStorage.hpp"

namespace RR::Ecs
{
    ArchetypeEntityIndex Archetype::Insert(EntityStorage& entityStorage, EntityId entityId)
    {
        auto index = componentsData.Insert();
        *(EntityId*)componentsData.GetData(ArchetypeComponentIndex(0), index) = entityId;
        entityStorage.Mutate(entityId, id, index);
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

           std::byte* dst = componentsData.GetData(ArchetypeComponentIndex(componentIndex), index);

           // TODO Could be faster find if we start from previous finded, to not iterate over same components id.
           auto fromComponentIndex = from.GetComponentIndex(componentInfo.id);

           if (!fromComponentIndex)
               continue;

           std::byte* src = from.componentsData.GetData(fromComponentIndex, fromIndex);
           componentInfo.move(dst, src);
        }

        entityStorage.Mutate(*(EntityId*)from.componentsData.GetData(ArchetypeComponentIndex(0), fromIndex), id, index);
        from.Delete(entityStorage, fromIndex, false);

        return index;
    }

    void Archetype::Delete(EntityStorage& entityStorage, ArchetypeEntityIndex index, bool updateEntityRecord)
    {
        const auto lastIndex = componentsData.GetLastIndex();

        if (index != lastIndex)
            entityStorage.Move(*(EntityId*)componentsData.GetData(ArchetypeComponentIndex(0), lastIndex), index);

        if (updateEntityRecord)
            entityStorage.Destroy(*(EntityId*)componentsData.GetData(ArchetypeComponentIndex(0), index));

        for (size_t componentIndex = 0; componentIndex < componentsData.componentsInfo.size(); componentIndex++)
        {
            const auto& componentInfo = componentsData.componentsInfo[componentIndex];
            if (componentInfo.size == 0)
                continue;

            const auto removedPtr = componentsData.GetData(ArchetypeComponentIndex(componentIndex), index);
            const auto lastIndexData = componentsData.GetData(ArchetypeComponentIndex(componentIndex), lastIndex);

            if (index != lastIndex)
                componentInfo.move(removedPtr, lastIndexData);

            if (componentInfo.destructor)
                componentInfo.destructor(lastIndexData);
        }

        componentsData.entitiesCount--;
    }
}