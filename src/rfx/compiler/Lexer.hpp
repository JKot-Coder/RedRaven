#pragma once

#include "core/SourceLocation.hpp"

#include "compiler/InputString.hpp"
#include "compiler/Token.hpp"

#include "common/EnumClassOperators.hpp"

namespace RR
{
    namespace Common
    {
        class LinearAllocator;
    }

    namespace Rfx
    {
        class DiagnosticSink;

        class Lexer final
        {
        public:
            static constexpr U8Glyph kEOF = 0xFFFFFF;

            Lexer() = delete;
            Lexer(const std::shared_ptr<SourceView>& sourceView, const std::shared_ptr<DiagnosticSink>& diagnosticSink);
            ~Lexer();

            Token GetNextToken();
            std::shared_ptr<std::vector<Token>> LexAllSemanticTokens();

        private:
            enum class Flags : uint32_t
            {
                None = 0 << 0,
                AtStartOfLine = 1 << 0,
                AfterWhitespace = 1 << 1,
                EscapedNewLines = 1 << 2
            };
            ENUM_CLASS_FRIEND_OPERATORS(Flags)

            class Counter final
            {
            public:
                Counter(uint32_t initialValue) : counter_(initialValue) {};
                inline void Reset(uint32_t initialValue = 0) { counter_ = initialValue; }
                inline void Increment() { counter_++; }
                inline uint32_t Value() const { return counter_; }

            private:
                uint32_t counter_ = 0;
            };

        private:
            inline bool isReachEOF()
            {
                return cursor_ == end_;
            }

            inline U8Glyph peek()
            {
                if (isReachEOF())
                    return kEOF;

                return utf8::peek_next(cursor_, end_);
            }

            TokenType scanToken();
            void advance();
            SourceLocation getSourceLocation();
            HumaneSourceLocation getHumaneSourceLocation();

            void handleBlockComment();
            void handleEscapedNewline();
            void handleLineComment();
            void handleNewlineSequence();
            void handleWhiteSpace();

            TokenType lexNumber(uint32_t base);
            bool maybeLexNumberExponent(uint32_t base);
            void lexDigits(uint32_t base);
            TokenType lexDirective();
            void lexIdentifier();
            void lexNumberAfterDecimalPoint(uint32_t base);
            void lexNumberSuffix();
            void lexStringLiteralBody(U8Glyph quote);

        private:
            const char* cursor_;
            const char* begin_;
            const char* end_;
            Counter linesCounter_ = 1;
            Counter columnCounter_ = 1;
            Flags flags_ = Flags::AtStartOfLine;

            std::unique_ptr<LinearAllocator> allocator_;
            std::shared_ptr<SourceView> sourceView_;
            std::shared_ptr<DiagnosticSink> sink_;
        };
    }
}