#pragma once

#include "parse_tools/core/SourceLocation.hpp"
#include "parse_tools/Token.hpp"
#include "utf8.h"

namespace RR
{
    namespace ParseTools
    {
        class DiagnosticSink;
        struct CompileContext;

        class Lexer final : Common::NonCopyable
        {
        public:
            static constexpr char32_t kEOF = 0xFFFFFF;

            enum class Flags : uint32_t
            {
                None = 0 << 0,
                SuppressDiagnostics = 1 << 0,
            };
            ENUM_CLASS_FRIEND_BITWISE_OPS(Flags)

        public:
            Lexer(const std::shared_ptr<SourceView>& sourceView, const std::shared_ptr<CompileContext>& context);
            ~Lexer();

            std::shared_ptr<SourceView> GetSourceView() const { return sourceView_; }
            DiagnosticSink& GetDiagnosticSink() const;

            Flags GetLexerFlags() const { return lexerFlags_; }
            void EnableFlags(Flags flags) { lexerFlags_ |= flags; }
            void DisableFlags(Flags flags) { lexerFlags_ &= ~flags; }

            Token ReadToken();
            TokenList LexAllSemanticTokens();

        private:
            class Counter final
            {
            public:
                Counter(uint32_t initialValue) : counter_(initialValue) {};
                inline void Reset(uint32_t initialValue = 0) { counter_ = initialValue; }
                inline void Increment() { counter_++; }
                inline void Increment(uint32_t value) { counter_ += value; }
                inline uint32_t Value() const { return counter_; }

            private:
                uint32_t counter_ = 0;
            };

        private:
            auto& getAllocator();

            inline bool isReachEOF() const { return cursor_ == end_; }

            inline char32_t peek() const
            {
                if (isReachEOF())
                    return kEOF;

                return utf8::peek_next(cursor_, end_);
            }

            Token::Type scanToken();
            void advance();
            SourceLocation getSourceLocation();

            void handleBlockComment();
            void handleEscapedNewline();
            void handleLineComment();
            void handleNewlineSequence();
            void handleWhiteSpace();

            Token::Type lexNumber(uint32_t base);
            bool maybeLexNumberExponent(uint32_t base);
            void lexDigits(uint32_t base);
            void lexIdentifier();
            void lexNumberAfterDecimalPoint(uint32_t base);
            void lexNumberSuffix();
            void lexStringLiteralBody(char32_t quote);

        private:
            std::shared_ptr<SourceView> sourceView_;
            std::shared_ptr<CompileContext> context_;
            using const_iterator = UnownedStringSlice::const_iterator;

            const_iterator cursor_;
            const_iterator begin_;
            const_iterator end_;
            Counter linesCounter_ = 1;
            Counter columnCounter_ = 1;
            Token::Flags tokenflags_ = Token::Flags::AtStartOfLine;
            Flags lexerFlags_ = Flags::None;
        };
    }
}