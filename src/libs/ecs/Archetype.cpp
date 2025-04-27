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

           std::byte* dst = componentsData.GetComponentData(ArchetypeComponentIndex(componentIndex), index);

           // TODO Could be faster find if we start from previous finded, to not iterate over same components id.
           auto fromComponentIndex = from.GetComponentIndex(componentInfo.id);

           if (!fromComponentIndex)
               continue;

           std::byte* src = from.componentsData.GetComponentData(fromComponentIndex, fromIndex);
           componentInfo.move(dst, src);
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

            auto *const removedPtr = componentsData.GetComponentData(ArchetypeComponentIndex(componentIndex), index);
            auto *const lastIndexData = componentsData.GetComponentData(ArchetypeComponentIndex(componentIndex), lastIndex);

            if (index != lastIndex)
                componentInfo.move(removedPtr, lastIndexData);

            if (componentInfo.destructor != nullptr)
                componentInfo.destructor(lastIndexData);
        }

        componentsData.entitiesCount--;
    }
}