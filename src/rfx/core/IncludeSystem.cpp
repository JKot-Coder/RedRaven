#include "IncludeSystem.hpp"

#include "core/SourceLocation.hpp"

#include <filesystem>

namespace RR
{
    namespace Rfx
    {
        RfxResult IncludeSystem::FindFile(const U8String& pathToInclude, const U8String& pathIncludedFrom, PathInfo& outPathInfo)
        {
            outPathInfo.type = PathInfo::Type::Unknown;

            // If it's absolute we only have to try and find if it's there - no need to look at search paths
            if (std::filesystem::path(pathToInclude).is_absolute())
            {
                // We pass in "" as the from path, so ensure no from path is taken into account
                // and to allow easy identification that this is in effect absolute
                return FindFile(PathType::Directory, "", pathToInclude, outPathInfo);
            }

            // Try just relative to current path
            {
                RfxResult result = FindFile(PathType::File, pathIncludedFrom, pathToInclude, outPathInfo);
                // It either succeeded or wasn't found, anything else is a failure passed back
                if (RFX_SUCCEEDED(result) || result != RfxResult::NotFound)
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

        RfxResult IncludeSystem::FindFile(PathType fromPathType, const U8String& fromPath, const U8String& path, PathInfo& outPathInfo)
        {
            U8String combinedPath;

            (void)fromPathType;
            (void)fromPath;
            (void)path;
            (void)outPathInfo;
            /*
                if (fromPath.length() == 0 || std::filesystem::path(path).is_absolute())
                {
                    // If the path is absolute or the fromPath is empty, the combined path is just the path
                    combinedPath = path;
                }
                else
                {
                    // Get relative path
                    SLANG_RETURN_ON_FAIL(m_fileSystemExt->calcCombinedPath(fromPathType, fromPath.begin(), path.begin(), combinedPathBlob.writeRef()));
                    combinedPath = StringUtil::getString(combinedPathBlob);
                    if (combinedPath.length() <= 0)
                        return RfxResult::Fail;
                }

                // This checks the path exists
                SlangPathType pathType;
                SLANG_RETURN_ON_FAIL(m_fileSystemExt->getPathType(combinedPath.begin(), &pathType));
                if (pathType != SLANG_PATH_TYPE_FILE)
                    return RfxResult::NotFound;

                // Get the uniqueIdentity
                SLANG_RETURN_ON_FAIL(m_fileSystemExt->getFileUniqueIdentity(combinedPath.begin(), uniqueIdentityBlob.writeRef()));

                // If the rel path exists -> a uniqueIdentity MUST exists too
                U8String uniqueIdentity(StringUtil::getString(uniqueIdentityBlob));
                if (uniqueIdentity.length() <= 0)
                    return RfxResult::Fail; // Unique identity can't be empty

                outPathInfo = PathInfo::makeNormal(combinedPath, uniqueIdentity);*/
            return RfxResult::Ok;
        }
    }
}