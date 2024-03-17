#include "Bundle.hpp"

#include "common/Result.hpp"

#include <exception>
#include <yaml-cpp/yaml.h>

namespace RR::AssetImporter::Processors
{
    REGISTER_PROCESSOR(Bundle)

    std::vector<U8String> Bundle::GetListOfExtensions() const
    {
        return { ".bundle" };
    }

    RR::Common::RResult Bundle::Process(const Asset& asset, const ProcessorContext& context, std::vector<ProcessorOutput>& outputs)const
    {
        std::ignore = asset;
        std::ignore = context;
        std::ignore = outputs;

        try
        {
            auto yaml = YAML::LoadFile(asset.path);

            auto version = yaml["version"].as<std::uint32_t>();
            if (version != 1)
                return RR::Common::RResult::Fail;

            auto bundleYaml = yaml["bundle"];
          //  bundle.name = bundleYaml["name"].as<std::string>();
         //   bundle.include_mask = bundleYaml["include_mask"].as<std::string>();
         //   bundle.exclude_mask = bundleYaml["exclude_mask"].as<std::string>();
        }
        catch (std::exception& e)
        {
            return RR::Common::RResult::Fail;
        }

        LOG_INFO("Process ", asset.path.c_str());

        return Common::RResult::Ok;
    }
}