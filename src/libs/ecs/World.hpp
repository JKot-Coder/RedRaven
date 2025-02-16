#pragma once

// #include "ecs/Event.hpp"
#include "ecs/ComponentTraits.hpp"
#include "ecs/ForwardDeclarations.hpp"
#include "ecs/FunctionTraits.hpp"
#include "ecs/System.hpp"
#include "ecs/TypeStorage.hpp"
#include "ska/flat_hash_map.h"

#include "eastl/vector_set.h"
#include "eastl/sort.h"
#include "flecs/flecs.h"

namespace RR::Ecs
{
    using EntityId = flecs::entity_t;

    template <typename Key, size_t ElementsCount, bool EnableOverflow = true>
    using FixedVectorSet = eastl::vector_set<Key, eastl::less<Key>, EASTLAllocatorType, eastl::fixed_vector<Key, ElementsCount, EnableOverflow>>;
    struct World
    {
    public:
        World()
        {
            world = ecs_init();
            ASSERT(world);
        }

        template <typename EventType>
        inline EventBuilder<EventType> Event() const;
        template <typename... Components>
        inline SystemBuilder<Components...> System(const char* name);
        EntityBuilder<void, void> Entity();

        template <typename T, typename DescriptionType>
        T Init(const DescriptionType& desc);

        void Tick();

        template <typename... Components>
        QueryBuilder Query();

        template <typename Callable>
        void Each(Callable&& callable)
        {
            using ArgList = GetArgumentList<Callable>;
            queryImpl<ArgList>(eastl::forward<Callable>(callable), eastl::make_index_sequence<ArgList::Count>());
        }

    private:
        template <typename EventType>
        friend struct EventBuilder;
        friend struct QueryBuilder;
        template <typename C, typename A>
        friend struct EntityBuilder;

       // EntityId createEntity() { return ecs_new(world); }

        template <typename Components, typename ArgsTuple>
        void commit(ArgsTuple&& args)
        {
            commitImpl<Components>(eastl::forward<ArgsTuple>(args), eastl::make_index_sequence<Components::Count>());
        }

        template <typename Components, typename ArgsTuple, size_t... Index>
        void commitImpl(ArgsTuple&& args, eastl::index_sequence<Index...>)
        {
            EntityId entity = ecs_new(world);
            ecs_record_t* record = ecs_record_find(world, entity);
            ASSERT(record);
            ecs_table_t* table = record ? record->table : nullptr;

            // Find destination table that has all components
            ecs_table_t *prev = table, *next;
            ecs_entity_t componentId = 0;
            size_t elem = 0;
            FixedVectorSet<ecs_entity_t, Components::Count, false> added;

            auto store_added = [&added](ecs_table_t* prev,
                                        ecs_table_t* next, ecs_id_t id) {
                // Array should only contain ids for components that are actually added,
                // so check if the prev and next tables are different.
                if (prev != next)
                    added.push_back_unsorted(id);
            };

            // Iterate components, only store added component ids in added array
            // clang-format off
            (void)std::initializer_list<int>({
                (componentId = getComponentId<typename Components::template Get<Index>>(),
                next = ecs_table_add_id(world, prev, componentId),
                store_added(prev, next, componentId),
                prev = next,
                0)...}
            );
            // clang-format on

            // If table is different, move entity straight to it
            if (table != next) {
                ecs_type_t addedIds;
                addedIds.array = added.data();
                addedIds.count = static_cast<ecs_size_t>(elem);
                ecs_commit(world, entity, record, next, &addedIds, nullptr);
                table = next;
            }
        }

        template <typename Component>
        ecs_id_t getComponentId()
        {
            auto Id = GetComponentId<Component>;
            auto it = componentsMap.find(Id);

            if (it != componentsMap.end())
                return it->second;

            ecs_entity_desc_t entityDesc {};
            entityDesc.name = GetComponentName<Component>;
            entityDesc.symbol = GetComponentName<Component>;
            entityDesc.use_low_id = true;

            ecs_component_desc_t componentDesc {};
            componentDesc.entity = ecs_entity_init(world, &entityDesc);
            componentDesc.type.size = ECS_SIZEOF(Component);
            componentDesc.type.alignment = ECS_ALIGNOF(Component);

            ecs_id_t componentId = ecs_component_init(world, &componentDesc);

            ASSERT_MSG(componentId != 0, "Failed to create component {}", componentId);
            componentsMap[Id] = componentId;
            return componentId;
        }

        template <typename Component, typename... Args>
        void add(EntityId entity, Args&&... args)
        {
            ecs_entity_t componentId = getComponentId<Component>();
            Component& dst = *static_cast<Component*>(ecs_emplace_id(world, entity, componentId, nullptr));
            FLECS_PLACEMENT_NEW(&dst, Component {FLECS_FWD(args)...});
            ecs_modified_id(world, entity, componentId);
        }

        template <typename ArgumentList, typename Callable, size_t... Index>
        void queryImpl(Callable&& callable, eastl::index_sequence<Index...>)
        {
            ecs_query_desc_t query_desc {};
            eastl::array<ecs_term_t, ArgumentList::Count> args = {};

            // Initialize each term separately
            (void)std::initializer_list<int>{(
                args[Index].id = getComponentId<typename ArgumentList::template Get<Index>>(),
                0)...};

            // Assign terms to query
            (void)std::initializer_list<int>{(
                query_desc.terms[Index] = args[Index],
                0)...};

            query_desc.cache_kind = EcsQueryCacheNone;
            query_desc.flags = EcsQueryMatchEmptyTables;

            ecs_query_t* query = ecs_query_init(world, &query_desc);
            ecs_iter_t it = ecs_query_iter(world, query);

            // Outer loop: matching tables
            while (ecs_query_next(&it))
            {
                for (int32_t i = 0; i < it.count; i++)
                {
                    callable(static_cast<typename ArgumentList::template Get<Index>>(
                        *(ecs_field(&it, typename ArgumentList::template Get<Index>, Index) + i))...);
                }
            }

            ecs_query_fini(query);
        }

        ecs_world_t* flecs() { return world; }

        /*
                template <typename EventType>
                void emit(EventType&& event, const EventDescription& eventDesc) const;
                template <typename EventType>
                void emitImmediately(EventType&& event, const EventDescription& eventDesc) const;
        */
        void broadcastEventImmediately(const Ecs::Event& event) const;
        void dispatchEventImmediately(EntityId entity, const Ecs::Event& event) const;

    private:
        ecs_world_t* world;
        //  EventStorage eventStorage;
        SystemStorage systemStorage;
        TypeStorage typeStorage;
        ska::flat_hash_map<ComponentId, ecs_id_t> componentsMap;
    };
    /*
        template <typename EventType>
        void World::emit(EventType&& event, const EventDescription& eventDesc) const
        {
            static_assert(std::is_base_of<Ecs::event, EventType>::value, "EventType must derive from Event");
            eventStorage.Push(std::move(event), eventDesc);
        }

        template <typename EventType>
        void World::emitImmediately(EventType&& event, const EventDescription& eventDesc) const
        {
            static_assert(std::is_base_of<Ecs::Event, EventType>::value, "EventType must derive from Event");
            if (!eventDesc.entity)
                broadcastEventImmediately(std::move(event));
            else
                dispatchEventImmediately(eventDesc.entity, std::move(event));
        }*/

    template <>
    inline System World::Init<System>(const SystemDescription& desc)
    {
        systemStorage.Push(desc);
        return Ecs::System(*this, desc.hashName);
    }

}