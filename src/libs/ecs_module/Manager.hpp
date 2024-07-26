#pragma once

#include "ecs_module\Module.hpp"

namespace RR::Common
{
    enum class RResult : int32_t;
}

namespace flecs {
    struct world;
}

namespace RR::EcsModule
{
    class Manager final
    {
    public:
        Common::RResult Load(const std::string& path, flecs::world& world);

    private:
        std::vector<Module> modules_;
    };
}