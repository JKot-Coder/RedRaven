#pragma once

#include "ecs/ForwardDeclarations.hpp"
#include "ecs/ComponentTraits.hpp"

namespace RR::Ecs
{
    struct QueryDescription
    {
        ComponentsSet require;
        ComponentsSet exclude;
    };

    struct Query
    {
        ~Query()
        {
        }

        template <typename Callable>
        void Each(Callable&& callable)
        {
            world_.query(*this, eastl::forward<Callable>(callable));
        }

    private:
        friend World;

        explicit Query(World& world, QueryDescription&& desc_);

    private:
        auto& desc() { return desc_; };
        const auto& desc() const { return desc_; };
/* 
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
        }*/

    private:
        World& world_;
        QueryDescription desc_;
    };

    struct QueryBuilder
    {
        Query Build();

        template <typename... Components>
        QueryBuilder Require()
        {
            (require(GetComponentId<Components>), ...);
            return *this;
        }

    private:
        auto& desc() { return desc_; };

    private:
        friend World;

        void require(ComponentId id) { desc().require.push_back_unsorted(id); }

        explicit QueryBuilder(World& world) : world_ {world}
        {
        //    desc_.cache_kind = EcsQueryCacheNone;
        //    desc_.flags = EcsQueryMatchEmptyTables;
        };


    private:
        QueryDescription desc_ {};
        size_t termIndex_ = 0;
        World& world_;
    };
}
