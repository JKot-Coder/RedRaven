#pragma once

#include <unordered_map>

#include "core/SourceLocation.hpp"

namespace RR::Common
{
    enum class RResult;
}

namespace RR::Rfx
{
    using RResult = Common::RResult;

    struct CompileContext;

    class SourceManager final
    {
    public:
        SourceManager(const std::shared_ptr<CompileContext>& context);
        ~SourceManager();

        std::shared_ptr<SourceFile> CreateFileFromString(const PathInfo& pathInfo, const U8String& content);
        RResult LoadFile(const PathInfo& pathInfo, std::shared_ptr<SourceFile>& sourceFile);

        std::shared_ptr<SourceView> SourceManager::CreateSourceView(const std::shared_ptr<SourceFile>& sourceFile)
        {
            const auto sourceView = SourceView::CreateFromSourceFile(sourceFile);
            sourceViews_.push_back(sourceView);

            return sourceView;
        }

        std::shared_ptr<SourceView> SourceManager::CreatePastedSourceView(const std::shared_ptr<SourceFile>& sourceFile, const SourceLocation& parentSourceLocation)
        {
            const auto sourceView = SourceView::CreatePasted(sourceFile, parentSourceLocation);
            sourceViews_.push_back(sourceView);

            return sourceView;
        }

        std::shared_ptr<SourceView> SourceManager::CreateIncluded(const std::shared_ptr<SourceFile>& sourceFile,
                                                                  const SourceLocation& parentSourceLocation)
        {
            const auto sourceView = SourceView::CreateIncluded(sourceFile, parentSourceLocation);
            sourceViews_.push_back(sourceView);

            return sourceView;
        }

        std::shared_ptr<SourceView> SourceManager::CreateSplited(const SourceLocation& splitLocation, const PathInfo& ownPathInfo)
        {
            const auto sourceView = SourceView::CreateSplited(splitLocation, ownPathInfo);
            sourceViews_.push_back(sourceView);

            return sourceView;
        }

    private:
        RResult readFile(const PathInfo& pathInfo, std::shared_ptr<SourceFile>& outSourceFile);
        RResult findSourceFileByUniqueIdentity(const U8String& uniqueIdentity, std::shared_ptr<SourceFile>& sourceFile) const;
        void addSourceFile(const U8String& uniqueIdentity, std::shared_ptr<SourceFile>& sourceFile);

    private:
        std::vector<std::shared_ptr<SourceFile>> sourceFiles_;
        std::vector<std::shared_ptr<SourceView>> sourceViews_;
        std::unordered_map<U8String, std::shared_ptr<SourceFile>> sourceFileMap_;
        std::shared_ptr<CompileContext> context_;
    };
}