#pragma once

namespace RR::Ecs
{
    struct World;
}

struct EcsFeatureRegistry
{
    using RegistrationFn = void (*)(RR::Ecs::World&);

    explicit EcsFeatureRegistry(RegistrationFn fn)
    {
        registry.push_back(fn);
    }

    template <typename Callable>
    static void ForEach(Callable&& callable)
    {
        for (auto& fn : registry)
            callable(fn);
    }

    inline static std::vector<RegistrationFn> registry;
};

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

#define ECS_REGISTER_FEATURE(function)                              \
    namespace                                                       \
    {                                                               \
        EcsFeatureRegistry CONCAT(function, __COUNTER__)(function); \
    }
