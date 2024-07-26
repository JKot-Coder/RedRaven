#include "Module.hpp"

#include "common/io/FileSystem.hpp"
#include "common/Result.hpp"
#include "flecs.h"

#define CR_HOST CR_SAFE
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
#include "cr.h"
#pragma clang diagnostic pop

namespace RR::EcsModule
{
    Module::Module(){}
    Module::~Module(){}

    Common::RResult Module::Load(const std::string& filename, flecs::world& world)
    {
        Common::IO::FileSystem fs;

        if(!fs.IsExist(filename))
            return Common::RResult::FileNotFound;

        std::unique_ptr<cr_plugin> impl = std::make_unique<cr_plugin>();

        if(!cr_plugin_open(*impl, filename.c_str()))
            return Common::RResult::Fail;

        impl->userdata = &world;

        cr_plugin_reload(*impl);

        impl_.swap(impl);

        return Common::RResult::Ok;
    }
}