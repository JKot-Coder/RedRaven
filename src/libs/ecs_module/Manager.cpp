#include "Manager.hpp"

#include "common\Result.hpp"
#include "cr.h"
#include "flecs.h"

namespace RR::EcsModule
{
    Common::RResult Manager::Load(const std::string& path)
    {
        Module module;

        Common::RResult result = module.Load(path, ctx_);
        if(result != Common::RResult::Ok)
        {
            Log::Format::Error("Failed to load module: {}. Error: {}", path, Common::GetErrorMessage(result));
            return result;
        }

        modules_.push_back(std::move(module));
        return Common::RResult::Ok;
    }

    void Manager::Update()
    {
        for(auto& module : modules_)
        {
            module.Reload();
        }
    }
}