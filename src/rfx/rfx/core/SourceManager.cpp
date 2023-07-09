#include "SourceManager.hpp"

#include "common/Result.hpp"
#include "core/SourceLocation.hpp"

namespace RR::Rfx
{
    RResult SourceManager::findSourceFileByUniqueIdentity(const U8String& uniqueIdentity, std::shared_ptr<SourceFile>& sourceFile) const
    {
        const auto findIt = sourceFileMap_.find(uniqueIdentity);
        if (findIt != sourceFileMap_.end())
        {
            sourceFile = findIt->second;
            return RResult::Ok;
        }

        return RResult::NotFound;
    }

    void SourceManager::addSourceFile(const U8String& uniqueIdentity, std::shared_ptr<SourceFile>& sourceFile)
    {
        ASSERT(RR_FAILED(findSourceFileByUniqueIdentity(uniqueIdentity, sourceFile)));
        sourceFileMap_.emplace(uniqueIdentity, sourceFile);
    }

    RResult SourceManager::LoadFile(const U8String& uniqueIdentity, std::shared_ptr<SourceFile>& sourceFile)
    {
        RR_RETURN_ON_SUCESS(findSourceFileByUniqueIdentity(uniqueIdentity, sourceFile));
        RR_RETURN_ON_FAIL(includeSystem_->LoadFile(pathInfo, sourceFile));
        addSourceFile(sourceFile);
        return RResult::Ok;
    }
}