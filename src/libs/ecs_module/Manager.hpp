#pragma once

#include "ecs_module\Module.hpp"
#include "ecs_module\Context.hpp"

namespace RR::Common
{
    enum class RResult : int32_t;
}

namespace RR::EcsModule
{
    class Manager final
    {
    public:
        Manager(const Context& ctx) : ctx_(ctx) {};
        Common::RResult Load(const std::string& path);
        void Update();

    private:
        Context ctx_;
        std::vector<Module> modules_;
    };
}