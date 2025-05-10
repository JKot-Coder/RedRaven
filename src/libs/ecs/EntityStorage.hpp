#pragma once

#include "ecs/Archetype.hpp"
#include "ecs/EntityId.hpp"
#include "ecs/ForwardDeclarations.hpp"

namespace RR::Ecs
{
    struct EntityRecord
    {
    private:
        ArchetypeEntityIndex index;
        uint32_t generation;
        Archetype* archetype;
        Archetype* pendingArchetype;

    public:
        EntityRecord() = default;
        EntityRecord(uint32_t generation) : generation(generation) { }

        friend struct EntityStorage;

    public:
        bool IsAlive(bool pending) const { return pending ? pendingArchetype != nullptr : true; }
        Archetype* GetArchetype(bool pending) const
        {
            return pending ? pendingArchetype : archetype;
        }
        ArchetypeEntityIndex GetIndex(bool pending) const
        {
            ASSERT(!pending);
            UNUSED(pending);
            return index;
        }
        bool HasPendingChanges() const { return archetype != pendingArchetype; }
    };

    struct EntityStorage final
    {
    private:
        eastl::vector<EntityRecord> entityRecords;
        eastl::vector<EntityId> freeId;

        EntityRecord& getFreeRecord(EntityId& entityId)
        {
            if (freeId.empty())
            {
                entityId = EntityId(entityRecords.size(), 0);
                return entityRecords.emplace_back(0);
            }
            else
            {
                entityId = freeId.back();
                freeId.pop_back();
                entityRecords[entityId.GetIndex()] = EntityRecord(entityId.GetGeneration());
                return entityRecords[entityId.GetIndex()];
            }
        }

    public:
        EntityId Create(Archetype& archetype)
        {
            EntityId entityId;
            EntityRecord& record = getFreeRecord(entityId);
            record.archetype = &archetype;
            record.pendingArchetype = &archetype;
            return entityId;
        }

        EntityId CreateAsync(Archetype& pendingArchetype)
        {
            EntityId entityId;
            EntityRecord& record = getFreeRecord(entityId);
            record.archetype = nullptr;
            record.pendingArchetype = &pendingArchetype;
            return entityId;
        }

        bool CanAcesss(EntityId entityId) const
        {
            return entityId.fields.index < entityRecords.size() &&
                   entityRecords[entityId.fields.index].generation == entityId.fields.generation;
        }

        void Destroy(EntityId entityId)
        {
            if (CanAcesss(entityId))
            {
                auto& entityRecord = entityRecords[entityId.fields.index];
                uint32_t nextGeneration = entityRecord.generation + 1;
                if (nextGeneration < EntityId::MaxGenerations)
                    freeId.push_back(EntityId(entityId.fields.index, nextGeneration));
                entityRecord.generation = EntityId::MaxGenerations;
            }
        }

        bool PendingDestroy(EntityId entityId)
        {
            if (!CanAcesss(entityId))
                return false;

            auto& entityRecord = entityRecords[entityId.fields.index];
            entityRecord.pendingArchetype = nullptr;
            return true;
        }

        bool PendingMutate(EntityId entityId, Archetype& archetype)
        {
            if (!CanAcesss(entityId))
                return false;

            auto& entityRecord = entityRecords[entityId.fields.index];
            entityRecord.pendingArchetype = &archetype;
            return true;
        }

        bool Move(EntityId entityId, ArchetypeEntityIndex index)
        {
            if (!CanAcesss(entityId))
                return false;
            auto& entityRecord = entityRecords[entityId.fields.index];
            entityRecord.index = index;
            return true;
        }

        bool Mutate(EntityId entityId, Archetype& archetype, ArchetypeEntityIndex index)
        {
            if (!CanAcesss(entityId))
                return false;

            auto& entityRecord = entityRecords[entityId.fields.index];
            entityRecord.archetype = &archetype;
            entityRecord.pendingArchetype = &archetype;
            entityRecord.index = index;
            return true;
        }

        bool Get(EntityId entityId, EntityRecord& record) const
        {
            if (!CanAcesss(entityId))
                return false;

            record = entityRecords[entityId.fields.index];
            return true;
        }
    };
}