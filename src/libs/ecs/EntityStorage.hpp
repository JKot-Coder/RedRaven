#pragma once

#include "ecs/EntityId.hpp"

namespace RR::Ecs
{
    struct EntityRecord
    {
        uint32_t generation;
    };

    struct EntityStorage
    {
        eastl::vector<EntityRecord> entityRecords;
        eastl::vector<uint32_t> freeIndices;

        EntityId Create()
        {
            EntityId entityId;
            if (freeIndices.empty())
            {
                entityId = EntityId(entityRecords.size(), 0);
                entityRecords.push_back({entityId.GetGeneration()});
            }
            else
            {
                uint32_t entityIndex = freeIndices.back();
                freeIndices.pop_back();

                uint32_t generation = entityRecords[entityIndex].generation;
                entityId = EntityId(entityIndex, generation);
                entityRecords[entityIndex] = {generation};
            }
            return entityId;
        }

        bool IsAlive(EntityId entityId) const
        {
            return entityId.GetEntityIndex() < entityRecords.size() && entityRecords[entityId.GetEntityIndex()].generation == entityId.GetGeneration();
        }

        void Destroy(EntityId entityId)
        {
            if (IsAlive(entityId))
            {
                auto& entityRecord = entityRecords[entityId.GetEntityIndex()];
                entityRecord.generation = (entityRecord.generation + 1) & EntityId::GenerationsMask;
                freeIndices.push_back(entityId.GetEntityIndex());
            }
        }
    };
}