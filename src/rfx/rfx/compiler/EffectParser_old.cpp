#include "EffectParser.hpp"

#include "core/FileSystem.hpp"
#include "core/IncludeSystem.hpp"
#include "core/SourceLocation.hpp"

#include "common/Result.hpp"

#include <yaml-cpp/yaml.h>
#include <iostream>

namespace RR::Rfx
{
    RfxResult EffectParser::Parse(const std::filesystem::path& path)
    {
        std::ignore = path;

        RfxResult result = RfxResult::Ok;

        PathInfo pathInfo;

        const auto& fileSystem = std::make_shared<OSFileSystem>();
        const auto& includeSystem = std::make_shared<IncludeSystem>(fileSystem);
        if (RR_FAILED(result = includeSystem->FindFile(path.u8string(), "", pathInfo)))
            return result;

        std::shared_ptr<RR::Rfx::SourceFile> sourceFile;
        if (RR_FAILED(result = includeSystem->LoadFile(pathInfo, sourceFile)))
            return result;

        auto stream = sourceFile->GetStream();
        const auto test = YAML::Load(stream);

        std::cout  << test << "\n";

        return result;
    }
}