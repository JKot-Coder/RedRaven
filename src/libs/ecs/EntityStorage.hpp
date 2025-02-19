#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/EntityId.hpp"

namespace RR::Ecs
{
    enum class EntityState
    {
        Dead,
        Alive,
        AsyncCreation,
        AsyncDestroy,
    };

    struct EntityRecord
    {
        EntityRecord() = default;
        EntityRecord(ArchetypeId archetypeId, ArchetypeEntityIndex index, uint32_t generation, EntityState state)
            : archetypeId(archetypeId), index(index), generation(generation), state(state)
        {
        }

        ArchetypeId archetypeId;
        ArchetypeEntityIndex index;
        uint32_t generation;
        EntityState state;
    };

    struct EntityStorage
    {
        eastl::vector<EntityRecord> entityRecords;
        eastl::vector<uint32_t> freeIndices;

        EntityId Create(EntityState state = EntityState::Alive)
        {
            EntityId entityId;
            if (freeIndices.empty())
            {
                entityId = EntityId(entityRecords.size(), 0);
                uint32_t generation = entityId.fields.generation;
                entityRecords.emplace_back(ArchetypeId::Invalid(), ArchetypeEntityIndex::Invalid(), generation, state);
            }
            else
            {
                uint32_t entityIndex = freeIndices.back();
                freeIndices.pop_back();

                uint32_t generation = entityRecords[entityIndex].generation;
                entityId = EntityId(entityIndex, generation);
                entityRecords[entityIndex] = {ArchetypeId::Invalid(), ArchetypeEntityIndex::Invalid(), generation, state};
            }
            return entityId;
        }

        bool IsAlive(EntityId entityId) const
        {
            return entityId.fields.index < entityRecords.size() &&
                   entityRecords[entityId.fields.index].generation == entityId.fields.generation &&
                   (entityRecords[entityId.fields.index].state == EntityState::Alive ||
                    entityRecords[entityId.fields.index].state == EntityState::AsyncCreation);
        }

        bool CanAcesss(EntityId entityId) const
        {
            return entityId.fields.index < entityRecords.size() &&
                   entityRecords[entityId.fields.index].generation == entityId.fields.generation &&
                   (entityRecords[entityId.fields.index].state == EntityState::Alive ||
                    entityRecords[entityId.fields.index].state == EntityState::AsyncDestroy);
        }

        void Destroy(EntityId entityId)
        {
            if (IsAlive(entityId))
            {
                auto& entityRecord = entityRecords[entityId.fields.index];
                entityRecord.generation = (entityRecord.generation + 1) & EntityId::GenerationsMask;
                freeIndices.push_back(entityId.fields.index);
            }
        }

        bool Move(EntityId entityId, ArchetypeEntityIndex index)
        {
            if (IsAlive(entityId))
            {
                auto& entityRecord = entityRecords[entityId.fields.index];
                entityRecord.index = index;
                entityRecord.state = EntityState::Alive;
                return true;
            }
            return false;
        }

        bool Mutate(EntityId entityId, ArchetypeId archetypeId, ArchetypeEntityIndex index)
        {
            if (IsAlive(entityId))
            {
                auto& entityRecord = entityRecords[entityId.fields.index];
                entityRecord.archetypeId = archetypeId;
                entityRecord.index = index;
                entityRecord.state = EntityState::Alive;
                return true;
            }
            return false;
        }

        bool Get(EntityId entityId, EntityRecord& entityRecord) const
        {
            if (!CanAcesss(entityId))
                return false;

            entityRecord = entityRecords[entityId.fields.index];
            return true;
        }
    };
}