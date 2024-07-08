#pragma once

#include <unordered_map>

#include "rfx/core/SourceView.hpp"

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

        std::shared_ptr<SourceFile> CreateFileFromString(const PathInfo& pathInfo, const std::string& content);
        RResult LoadFile(const PathInfo& pathInfo, std::shared_ptr<SourceFile>& sourceFile);

        std::shared_ptr<SourceView> CreateSourceView(const std::shared_ptr<SourceFile>& sourceFile)
        {
            const auto sourceView = SourceView::CreateFromSourceFile(sourceFile);
            sourceViews_.push_back(sourceView);

            return sourceView;
        }

        std::shared_ptr<SourceView> CreatePastedSourceView(const std::shared_ptr<SourceFile>& sourceFile, const Token& initiatingToken)
        {
            const auto sourceView = SourceView::CreatePasted(sourceFile, initiatingToken);
            sourceViews_.push_back(sourceView);

            return sourceView;
        }

        std::shared_ptr<SourceView> CreatePastedSourceView(const std::shared_ptr<const SourceView>& parentSourceView, const Token& initiatingToken)
        {
            const auto sourceView = SourceView::CreatePasted(parentSourceView, initiatingToken);
            sourceViews_.push_back(sourceView);

            return sourceView;
        }

        std::shared_ptr<SourceView> CreateIncluded(const std::shared_ptr<SourceFile>& sourceFile,
                                                   const Token& initiatingToken)
        {
            const auto sourceView = SourceView::CreateIncluded(sourceFile, initiatingToken);
            sourceViews_.push_back(sourceView);

            return sourceView;
        }

        std::shared_ptr<SourceView> CreateSplited(const SourceLocation& splitLocation, const PathInfo& ownPathInfo)
        {
            const auto sourceView = SourceView::CreateSplited(splitLocation, ownPathInfo);
            sourceViews_.push_back(sourceView);

            return sourceView;
        }

    private:
        RResult readFile(const PathInfo& pathInfo, std::shared_ptr<SourceFile>& outSourceFile);
        RResult findSourceFileByUniqueIdentity(const std::string& uniqueIdentity, std::shared_ptr<SourceFile>& sourceFile) const;
        void addSourceFile(const std::string& uniqueIdentity, std::shared_ptr<SourceFile>& sourceFile);

    private:
        std::vector<std::shared_ptr<SourceFile>> sourceFiles_;
        std::vector<std::shared_ptr<SourceView>> sourceViews_;
        std::unordered_map<std::string, std::shared_ptr<SourceFile>> sourceFileMap_;
        std::shared_ptr<CompileContext> context_;
    };
}