#include "Manager.hpp"

#include "common\Result.hpp"
#include "cr.h"
#include "flecs.h"

namespace RR::EcsModule
{
    Common::RResult Manager::Load(const std::string& path, flecs::world& world)
    {
        Module module;

        Common::RResult result = module.Load(path, world);
        if(result != Common::RResult::Ok)
        {
            Log::Format::Error("Failed to load module: {}. Error: {}", path, Common::GetErrorMessage(result));
            return result;
        }

        modules_.push_back(std::move(module));
        return Common::RResult::Ok;
    }
}