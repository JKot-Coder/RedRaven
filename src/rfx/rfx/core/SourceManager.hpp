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
        std::shared_ptr<SourceFile> CreateFileFromString(const PathInfo& pathInfo, const U8String& content);
        RResult LoadFile(const PathInfo& pathInfo, std::shared_ptr<SourceFile>& sourceFile);

    private:
        RResult findSourceFileByUniqueIdentity(const U8String& uniqueIdentity, std::shared_ptr<SourceFile>& sourceFile) const;
        void addSourceFile(const U8String& uniqueIdentity, std::shared_ptr<SourceFile>& sourceFile);

    private:
        std::vector<std::shared_ptr<SourceFile>> sourceFiles_;
        std::unordered_map<U8String, std::shared_ptr<SourceFile>> sourceFileMap_;
    };
}