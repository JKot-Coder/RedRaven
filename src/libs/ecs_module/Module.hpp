#pragma once

struct cr_plugin;

namespace flecs {
    struct world;
}

namespace RR::Common
{
    enum class RResult : int32_t;
}

namespace RR::EcsModule
{
    using ModuleImpl = cr_plugin;

    class Module final
    {
    public:
        Module();
        ~Module();

        Module(Module&&) noexcept = default;
        Module& operator=(Module&&) noexcept = default;

        Common::RResult Load(const std::string& filename, flecs::world& world);

    private:
        std::unique_ptr<ModuleImpl> impl_;
    };
}