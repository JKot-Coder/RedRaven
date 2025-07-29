#pragma once

#include "parse_tools/Token.hpp"
#include "parse_tools/core/SourceLocation.hpp"
#include "parse_tools/core/UnownedStringSlice.hpp"
#include "utf8.h"

namespace RR::ParseTools
{
    // A logical or physical storage object for a range of input code
    // that has logically contiguous source locations.
    class SourceFile final
    {
    public:
        SourceFile(const PathInfo& pathInfo) : pathInfo_(pathInfo) {};
        ~SourceFile() = default;

        UnownedStringSlice GetContent() const { return content_; }
        size_t GetContentSize() const { return content_.length(); }
        const PathInfo& GetPathInfo() const { return pathInfo_; }
        void SetContent(UnownedStringSlice content);

        /// Calculate a display path -> can canonicalize if necessary
        std::string CalcVerbosePath() const;

    private:
        UnownedStringSlice content_; ///< The actual contents of the file.
        PathInfo pathInfo_; ///< The path The logical file path to report for locations inside this span.
    };

    /* A SourceView maps to a single span of SourceLocation range and is equivalent to a single include or more precisely use of a source file.
    It is distinct from a SourceFile - because a SourceFile may be included multiple times, with different interpretations (depending
    on #defines for example). */
    class SourceView : public std::enable_shared_from_this<SourceView>, Common::NonCopyable
    {
    public:
        /// Get the source file holds the contents this view
        std::shared_ptr<SourceFile> GetSourceFile() const { return sourceFile_.lock(); }

        /// Get the associated 'content' (the source text)
        UnownedStringSlice GetContent() const { return content_; }

        UnownedStringSlice::const_iterator GetContentFrom(const SourceLocation& loc) const;

        size_t GetContentSize() const { return content_.length(); }

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

    protected:
        SourceView(const std::shared_ptr<SourceFile>& sourceFile,
                   UnownedStringSlice content,
                   const PathInfo& pathInfo,
                   const Token& initiatingToken)
            : sourceFile_(sourceFile), content_(content), pathInfo_(pathInfo), initiatingToken_(initiatingToken)
        {
            ASSERT(sourceFile); // TODO validate file
        }

    private:
        void advanceBom()
        {
            auto begin = content_.begin();
            const auto end = content_.end();

            if (utf8::starts_with_bom(begin, end))
                utf8::next(begin, end);

            content_ = UnownedStringSlice(begin, end);
        }

        template <typename T>
        struct MakeShared : public T
        {
            template <typename... Args>
            MakeShared(Args&&... args) : T(std::forward<Args>(args)...) { }

            template <typename... Args>
            static constexpr std::shared_ptr<T> Create(Args&&... args) { return std::make_shared<MakeShared<T>>(std::forward<Args>(args)...); }
        };

    public:
        [[nodiscard]]
        static std::shared_ptr<SourceView> CreateFromSourceFile(const std::shared_ptr<SourceFile>& sourceFile)
        {
            auto sourceView = MakeShared<SourceView>::Create(sourceFile, sourceFile->GetContent(), sourceFile->GetPathInfo(), Token {});
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
            auto sourceView = MakeShared<SourceView>::Create(sourceFile, sourceFile->GetContent(), pathInfo, initiatingToken);
            sourceView->advanceBom();

            return sourceView;
        }

        [[nodiscard]]
        static std::shared_ptr<SourceView> CreatePasted(const std::shared_ptr<const SourceView>& parentSourceView,
                                                        const Token& initiatingToken)
        {
            ASSERT(parentSourceView);

            const auto& pathInfo = PathInfo::makeTokenPaste();
            auto sourceView = MakeShared<SourceView>::Create(parentSourceView->GetSourceFile(), parentSourceView->GetContent(), pathInfo, initiatingToken);
            sourceView->advanceBom();

            return sourceView;
        }

        [[nodiscard]]
        static std::shared_ptr<SourceView> CreateIncluded(const std::shared_ptr<SourceFile>& sourceFile,
                                                          const Token& initiatingToken)
        {
            ASSERT(sourceFile);

            const auto& pathInfo = sourceFile->GetPathInfo();
            auto sourceView = MakeShared<SourceView>::Create(sourceFile, sourceFile->GetContent(), pathInfo, initiatingToken);
            sourceView->advanceBom();

            return sourceView;
        }

        [[nodiscard]]
        static std::shared_ptr<SourceView> CreateSplited(const SourceLocation& splitLocation, const PathInfo& ownPathInfo)
        {
            const auto sourceView = splitLocation.GetSourceView();
            ASSERT(sourceView);

            const auto content = UnownedStringSlice(sourceView->GetContentFrom(splitLocation), sourceView->GetSourceFile()->GetContent().end());
            const Token initiatingToken(Token::Type::Unknown, content, splitLocation);

            return MakeShared<SourceView>::Create(sourceView->GetSourceFile(), content, ownPathInfo, initiatingToken);
        }

    private:
        std::weak_ptr<SourceFile> sourceFile_; ///< The source file. Can hold the line breaks
        UnownedStringSlice content_;
        PathInfo pathInfo_;
        Token initiatingToken_; ///< An optional source loc that defines where this view was initiated from. SourceLocation(0) if not defined.
    };
}