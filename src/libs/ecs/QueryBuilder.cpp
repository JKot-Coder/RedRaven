#include "QueryBuilder.hpp"

#include "ecs/World.hpp"

namespace RR::Ecs
{
    Query::Query(World& world, QueryDescription&& desc_) : world_(world), desc_(eastl::move(desc_))
    {
    }

    Query QueryBuilder::Build()
    {
        eastl::quick_sort(desc().require.begin(), desc().require.end());
        return world_.Create<Query>(eastl::move(desc_));
    }
}
