#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/World.hpp"
#include "flecs.h"

namespace RR::Ecs
{
    struct Query
    {
        Query(ecs_world_t& world, ecs_query_t& query) : world_ {world}, query_(&query) { };

        ~Query()
        {
            /* Only free if query is not associated with entity, such as system
             * queries and named queries. Named queries have to be either explicitly
             * deleted with the .destruct() method, or will be deleted when the
             * world is deleted. */
            if (query_ && !query_->entity)
            {
                if (!flecs_poly_release(query_))
                {
                    ecs_query_fini(query_);
                    query_ = nullptr;
                }
            }
        }

        template <typename Callable>
        void Each(Callable&& callable)
        {
            using ArgList = GetArgumentList<Callable>;
            queryImpl<ArgList>(eastl::forward<Callable>(callable), eastl::make_index_sequence<ArgList::Count>());
        }

    private:
        template <typename ArgumentList, typename Callable, size_t... Index>
        void queryImpl(Callable&& callable, eastl::index_sequence<Index...>)
        {
            ecs_iter_t it = ecs_query_iter(&world_, query_);

            // Outer loop: matching tables
            while (ecs_query_next(&it))
            {
                LOG_ERROR("qweqwe");
                for (int32_t i = 0; i < it.count; i++)
                {
                    callable(static_cast<typename ArgumentList::template Get<Index>>(
                        *(ecs_field(&it, typename ArgumentList::template Get<Index>, Index) + i))...);
                }
            }
        }

    private:
        ecs_world_t& world_;
        ecs_query_t* query_ = nullptr;
    };

    struct QueryBuilder
    {
        Query Build()
        {
            ecs_query_t* query = ecs_query_init(world_.flecs(), &desc_);
            ASSERT(query);
            return Query(*world_.flecs(), *query);
        }

        void Cache() { desc_.cache_kind = EcsQueryCacheAuto; }

        template <typename... Components>
        QueryBuilder With()
        {
            ([&] {
                ecs_term_t term {};
                term.id = world_.getComponentId<Components>();
                addTerm(std::move(term));
            }(),
             ...);
            return *this;
        }

    private:
        friend World;

        void addTerm(ecs_term_t&& term)
        {
            ASSERT(ecs_term_is_initialized(&term));
            ASSERT(term_index_ < FLECS_TERM_COUNT_MAX);
            desc_.terms[term_index_] = eastl::move(term);
            term_index_++;
        }

        explicit QueryBuilder(World& world) : world_ {world}
        {
            desc_.cache_kind = EcsQueryCacheNone;
            desc_.flags = EcsQueryMatchEmptyTables;
        };

    private:
        size_t term_index_ = 0;
        ecs_query_desc_t desc_ {};
        World& world_;
    };

    template <typename... Components>
    QueryBuilder World::Query() { return QueryBuilder(*this).With<Components...>(); }
}
