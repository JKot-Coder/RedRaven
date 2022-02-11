#pragma once

#include "core/SourceLocation.hpp"

#include "compiler/InputString.hpp"
#include "compiler/Token.hpp"

namespace RR
{
    namespace Common
    {
        class LinearAllocator;
    }

    namespace Rfx
    {
        class DiagnosticSink;

        using TokenList = std::vector<Token>;

        struct TokenSpan
        {
        public:
            TokenSpan() = default;
            TokenSpan(const TokenList& tokenList)
                : begin_(tokenList.begin()), end_(tokenList.end()) { }

            TokenList::const_iterator Begin() const { return begin_; }
            TokenList::const_iterator End() const { return end_; }
            size_t GetSize() { return std::distance(end_, begin_); }

        private:
            TokenList::const_iterator begin_;
            TokenList::const_iterator end_;
        };

        struct TokenReader
        {
            using const_iterator = TokenList::const_iterator;

            TokenReader() = default;

            explicit TokenReader(const TokenSpan& tokens)
                : cursor_(tokens.Begin()), end_(tokens.End())
            {
                ASSERT(std::distance(cursor_, end_) > 0)
                ASSERT(end_->type == Token::Type::EndOfFile)

                updateLookaheadToken();
            }

            explicit TokenReader(const TokenList& tokens)
                : cursor_(tokens.begin()), end_(std::prev(tokens.end()))
            {
                ASSERT(std::distance(cursor_, end_) > 0)
                ASSERT(end_->type == Token::Type::EndOfFile)

                updateLookaheadToken();
            }

            explicit TokenReader(const_iterator begin, const_iterator end)
                : cursor_(begin), end_(end)
            {
                ASSERT(std::distance(cursor_, end_) > 0)
                ASSERT(end_->type == Token::Type::EndOfFile)

                updateLookaheadToken();
            }

            struct ParsingCursor
            {
                Token nextToken;
                const_iterator tokenReaderCursor;
            };

            ParsingCursor getCursor()
            {
                ParsingCursor rs;
                rs.nextToken = nextToken_;
                rs.tokenReaderCursor = cursor_;
                return rs;
            }

            void SetCursor(ParsingCursor cursor)
            {
                cursor_ = cursor.tokenReaderCursor;
                nextToken_ = cursor.nextToken;
            }

            bool IsAtCursor(const ParsingCursor& cursor) const
            {
                return cursor.tokenReaderCursor == cursor_;
            }

            bool IsAtEnd() const { return cursor_ == end_; }

            const Token& PeekToken() const
            {
                return nextToken_;
            }

            Token::Type PeekTokenType() const
            {
                return nextToken_.type;
            }

            //SourceLoc PeekLoc() const;

            Token AdvanceToken();

            size_t GetCount() { return std::distance(cursor_, end_); }

        private:
            /// Update the lookahead token in `m_nextToken` to reflect the cursor state
            void updateLookaheadToken();

        private:
            Token nextToken_;
            TokenList::const_iterator cursor_;
            TokenList::const_iterator end_;
            //static Token getEndOfFileToken();
        };

        class Lexer final
        {
        public:
            static constexpr U8Glyph kEOF = 0xFFFFFF;

            enum class Flags : uint32_t
            {
                None = 0 << 0,
                SuppressDiagnostics = 1 << 0,
            };
            ENUM_CLASS_FRIEND_OPERATORS(Flags)

        public:
            Lexer(const std::shared_ptr<SourceView>& sourceView, const std::shared_ptr<DiagnosticSink>& diagnosticSink);
            ~Lexer();

            std::shared_ptr<DiagnosticSink> GetDiagnosticSink() const { return sink_; }

            void EnableFlags(Flags flags) { lexerFlags_ |= flags; }
            void DisableFlags(Flags flags) { lexerFlags_ &= ~flags; }

            Flags GetLexerFlags() const { return lexerFlags_; }

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

            Token::Type scanToken();
            void advance();
            SourceLocation getSourceLocation();
            HumaneSourceLocation getHumaneSourceLocation();

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
            void lexStringLiteralBody(U8Glyph quote);

        private:
            std::unique_ptr<LinearAllocator> allocator_;
            std::shared_ptr<SourceView> sourceView_;
            std::shared_ptr<DiagnosticSink> sink_;

            const char* cursor_;
            const char* begin_;
            const char* end_;
            Counter linesCounter_ = 1;
            Counter columnCounter_ = 1;
            Token::Flags tokenflags_ = Token::Flags::AtStartOfLine;
            Flags lexerFlags_ = Flags::None;
        };
    }
}