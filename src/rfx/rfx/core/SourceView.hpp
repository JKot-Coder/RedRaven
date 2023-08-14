#pragma once

#include "rfx/core/UnownedStringSlice.hpp"
#include "rfx/core/SourceLocation.hpp"
#include "rfx/compiler/Token.hpp"

namespace RR::Rfx
{
    // A logical or physical storage object for a range of input code
    // that has logically contiguous source locations.
    class SourceFile final
    {
    public:
        SourceFile(const PathInfo& pathInfo) : pathInfo_(pathInfo) {};
        ~SourceFile() = default;

        UnownedStringSlice GetContent() const { return content_; }
        size_t GetContentSize() const { return content_.GetLength(); }
        const PathInfo& GetPathInfo() const { return pathInfo_; }
        void SetContent(const UnownedStringSlice& content);

        /// Calculate a display path -> can canonicalize if necessary
        U8String CalcVerbosePath() const;

    private:
        UnownedStringSlice content_; ///< The actual contents of the file.
        PathInfo pathInfo_; ///< The path The logical file path to report for locations inside this span.
    };

    /* A SourceView maps to a single span of SourceLocation range and is equivalent to a single include or more precisely use of a source file.
            It is distinct from a SourceFile - because a SourceFile may be included multiple times, with different interpretations (depending
            on #defines for example).
            */
    class SourceView final : public std::enable_shared_from_this<SourceView>, Common::NonCopyable
    {
    public:
        /// Get the source file holds the contents this view
        std::shared_ptr<SourceFile> GetSourceFile() const { return sourceFile_.lock(); }

        /// Get the associated 'content' (the source text)
        UnownedStringSlice GetContent() const { return content_; }

        const U8Char* GetContentFrom(const SourceLocation& loc) const;

        /// Gets the pathInfo for this view. It may be different from the m_sourceFile's if the path has been
        /// overridden by m_viewPath TODO COMMENT
        // IncludeStack GetIncludeStack() const { return includeStack_; }

        size_t GetContentSize() const { return content_.GetLength(); }

        SourceLocation GetSourceLocation(size_t offset, const HumaneSourceLocation& humaneSourceLoc) const
        {
            ASSERT(offset <= GetContentSize());
            return SourceLocation(offset, humaneSourceLoc, shared_from_this());
        }

        /// Gets the pathInfo for this view. It may differ from the pathInfo of the source file if the sourceView was created by #line directive.
        PathInfo GetPathInfo() const { return pathInfo_; }

        /// Get the humane location
        /// Type determines if the location wanted is the original, or the 'normal' (which modifys behavior based on #line directives)
        // HumaneSourceLocation GetHumaneLocation(const SourceLocation& loc, SourceLocationType type = SourceLocationType::Nominal)

        Token GetInitiatingToken() const { return initiatingToken_; }
        SourceLocation GetInitiatingSourceLocation() const { return initiatingToken_.sourceLocation; }
        UnownedStringSlice ExtractLineContainingLocation(const SourceLocation& loc) const;

    public:
        [[nodiscard]]
        static std::shared_ptr<SourceView> CreateFromSourceFile(const std::shared_ptr<SourceFile>& sourceFile)
        {
            auto sourceView = std::shared_ptr<SourceView>(new SourceView(sourceFile, sourceFile->GetContent(), sourceFile->GetPathInfo(), {}));
            sourceView->initiatingToken_.sourceLocation = SourceLocation(0, HumaneSourceLocation(1, 1), sourceView);
            sourceView->advanceBom();

            return sourceView;
        }

        [[nodiscard]]
        static std::shared_ptr<SourceView> CreatePasted(const std::shared_ptr<SourceFile>& sourceFile,
                                                        const Token& initiatingToken)
        {
            ASSERT(sourceFile);

            const auto& pathInfo = PathInfo::makeTokenPaste();
            auto sourceView = std::shared_ptr<SourceView>(new SourceView(sourceFile, sourceFile->GetContent(), pathInfo, initiatingToken));
            sourceView->advanceBom();

            return sourceView;
        }

        [[nodiscard]]
        static std::shared_ptr<SourceView> CreatePasted(const std::shared_ptr<const SourceView>& parentSourceView,
                                                        const Token& initiatingToken)
        {
            ASSERT(parentSourceView);

            const auto& pathInfo = PathInfo::makeTokenPaste();
            auto sourceView = std::shared_ptr<SourceView>(new SourceView(parentSourceView->GetSourceFile(), parentSourceView->GetContent(), pathInfo, initiatingToken));
            sourceView->advanceBom();

            return sourceView;
        }

        [[nodiscard]]
        static std::shared_ptr<SourceView> CreateIncluded(const std::shared_ptr<SourceFile>& sourceFile,
                                                          const Token& initiatingToken)
        {
            ASSERT(sourceFile);

            const auto& pathInfo = sourceFile->GetPathInfo();
            auto sourceView = std::shared_ptr<SourceView>(new SourceView(sourceFile, sourceFile->GetContent(), pathInfo, initiatingToken));
            sourceView->advanceBom();

            return sourceView;
        }

        [[nodiscard]]
        static std::shared_ptr<SourceView> CreateSplited(const SourceLocation& splitLocation, const PathInfo& ownPathInfo)
        {
            const auto sourceView = splitLocation.GetSourceView();
            ASSERT(sourceView);

            const auto content = UnownedStringSlice(sourceView->GetContentFrom(splitLocation), sourceView->GetSourceFile()->GetContent().End());
            const Token initiatingToken(Token::Type::Unknown, UnownedStringSlice(content.Begin(), content.Begin()), splitLocation);

            return std::shared_ptr<SourceView>(new SourceView(sourceView->GetSourceFile(), content, ownPathInfo, initiatingToken));
        }

    private:
        SourceView(const std::shared_ptr<SourceFile>& sourceFile,
                   const UnownedStringSlice& content,
                   const PathInfo& pathInfo,
                   const Token& initiatingToken)
            : sourceFile_(sourceFile), content_(content), pathInfo_(pathInfo), initiatingToken_(initiatingToken)
        {
            ASSERT(sourceFile); // TODO validate file
        }

        void advanceBom()
        {
            auto begin = content_.Begin();

            if (utf8::starts_with_bom(begin))
                utf8::next(begin, content_.End());

            content_ = UnownedStringSlice(begin, content_.End());
        }

        std::weak_ptr<SourceFile> sourceFile_; ///< The source file. Can hold the line breaks
        // IncludeStack includeStack_; ///< Path to this view. If empty the path is the path to the SourceView
        UnownedStringSlice content_;
        PathInfo pathInfo_;
        Token initiatingToken_; ///< An optional source loc that defines where this view was initiated from. SourceLocation(0) if not defined.
    };
}