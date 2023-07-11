#pragma once

#include <unordered_map>

namespace RR::Common
{
    enum class RResult;
}

namespace RR::Rfx
{
    class SourceFile;
    struct PathInfo;
    using RResult = Common::RResult;

    class SourceManager final
    {
    public:
        RResult LoadFile(const PathInfo& pathInfo, std::shared_ptr<SourceFile>& sourceFile);

    private:
        RResult findSourceFileByUniqueIdentity(const U8String& uniqueIdentity, std::shared_ptr<SourceFile>& sourceFile) const;
        void addSourceFile(const U8String& uniqueIdentity, std::shared_ptr<SourceFile>& sourceFile);

    private:
        std::unordered_map<U8String, std::shared_ptr<SourceFile>> sourceFileMap_;
    };
}