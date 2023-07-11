#include "AssetImporter.hpp"

#include "Processor.hpp"

#include "common/Result.hpp"

namespace RR::AssetImporter
{
    void AssetImporter::Register(Processor& processor)
    {
        for (const auto& extension : processor.GetListOfExtensions())
        {
            if (processors_.find(extension) != processors_.end())
                LOG_FATAL("Extension already registered");

            processors_.emplace(extension, &processor);
        }
    }

    void AssetImporter::importAsset(const std::filesystem::path& path)
    {
        auto metaPath = path;
        metaPath.append(".meta");

        if (!std::filesystem::exists(metaPath))
        {
        }
    }

    void AssetImporter::BuildBundle(const std::filesystem::path& path)
    {
        std::ignore = path;
        const auto bundleProcessor = processors_["bundle"];

        Asset asset { path };

        ProcessorContext context {};
        std::vector<ProcessorOutput> outputs;

        std::ignore = bundleProcessor->Process(asset, context, outputs);
    }

    Common::RResult AssetImporter::ProcessAsset(const std::filesystem::path& path)
    {
        const auto extension = path.extension().u8string();
        const auto processorIt = processors_.find(extension);

        if (processorIt == processors_.end())
        {
            LOG_WARNING("No processor found for file extension \"{0}\"", extension);
            return Common::RResult::NotImplemented;
        }

        ASSERT(processorIt->second);
        const auto& processor = *(processorIt->second);

        Asset asset { path };
        ProcessorContext context {};
        std::vector<ProcessorOutput> outputs;

        auto result = processor.Process(asset, context, outputs);
        if (RR_FAILED(result))
        {
            LOG_WARNING("Processing asset at path \"{0}\" failed with error: \"{1}\"", path.generic_u8string(), Common::GetErrorMessage(result));
        }

        return Common::RResult::Ok;
    }

}