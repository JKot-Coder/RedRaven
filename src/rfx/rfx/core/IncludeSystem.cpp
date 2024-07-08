#include "IncludeSystem.hpp"

#include "rfx/core/FileSystem.hpp"
#include "rfx/core/SourceLocation.hpp"

#include "common/LinearAllocator.hpp"
#include "common/OnScopeExit.hpp"
#include "common/Result.hpp"

#include <fstream>

namespace RR
{
    namespace Rfx
    {
        Common::RResult IncludeSystem::FindFile(const std::string& pathToInclude, const std::string& pathIncludedFrom, PathInfo& outPathInfo) const
        {
            outPathInfo.type = PathInfo::Type::Unknown;

            // If it's absolute we only have to try and find if it's there - no need to look at search paths
            if (fs::path(pathToInclude).is_absolute())
            {
                // We pass in "" as the from path, so ensure no from path is taken into account
                // and to allow easy identification that this is in effect absolute
                return findFileImpl("", pathToInclude, outPathInfo);
            }

            const auto includedFromStatus = fs::status(pathIncludedFrom);
            if (fs::is_regular_file(includedFromStatus) || fs::is_other(includedFromStatus))
            {
                return findFileImpl(pathToInclude, fs::path(pathIncludedFrom).parent_path(), outPathInfo);
            }

            // Try just relative to current path
            {
                RfxResult result = findFileImpl(pathToInclude, pathIncludedFrom, outPathInfo);
                // It either succeeded or wasn't found, anything else is a failure passed back
                if (RR_SUCCEEDED(result) || result != RfxResult::NotFound)
                    return result;
            }
            /*
                // Search all the searchDirectories
                for (auto sd = m_searchDirectories; sd; sd = sd->parent)
                {
                    for (auto& dir : sd->searchDirectories)
                    {
                        RfxResult result = findFile(SLANG_PATH_TYPE_DIRECTORY, dir.path, pathToInclude, outPathInfo);
                        if (RFX_SUCCEEDED(result) || result != RfxResult::NotFound)
                            return result;
                    }
                }*/

            return RfxResult::NotFound;
        }

        RfxResult IncludeSystem::findFileImpl(const fs::path& path, const fs::path& fromPath, PathInfo& outPathInfo) const
        {
            fs::path combinedPath;

            if (fromPath.empty() || path.is_absolute())
            {
                // If the path is absolute or the fromPath is empty, the combined path is just the path
                combinedPath = path;
            }
            else
            {
                // Get relative path
                combinedPath = fromPath / path;
            }

            // This checks the path exists
            fs::file_status status = fileSystemExt_->GetPathStatus(combinedPath);
            if (status.type() != fs::file_type::regular)
                return RfxResult::NotFound;

            // Get the uniqueIdentity
            std::string uniqueIdentity;
            RR_RETURN_ON_FAIL(fileSystemExt_->GetPathUniqueIdentity(combinedPath, uniqueIdentity));

            // If the rel path exists -> a uniqueIdentity MUST exists too
            if (uniqueIdentity.length() <= 0)
                return RfxResult::Fail; // Unique identity can't be empty

            outPathInfo = PathInfo::makeNormal(combinedPath.lexically_normal().generic_u8string(), uniqueIdentity);

            return RfxResult::Ok;
        }
    }
}