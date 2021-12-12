#include "Preprocessor.hpp"

#include "compiler/DiagnosticSink.hpp"
#include "compiler/Lexer.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            struct Preprocessor::InputFile
            {
                InputFile(
                    // Preprocessor* preprocessor,
                    // SourceView* sourceView
                )
                {
                }

                ~InputFile() = default;

                /// Is this input file skipping tokens (because the current location is inside a disabled condition)?
                //  bool isSkipping();

                /// Get the inner-most conditional that is in efffect at the current location
                // Conditional* getInnerMostConditional() { return m_conditional; }

                /// Push a new conditional onto the stack of conditionals in effect
                /* void pushConditional(Conditional* conditional)
                {
                    conditional->parent = m_conditional;
                    m_conditional = conditional;
                }*/

                /// Pop the inner-most conditional
                /* void popConditional()
                {
                    auto conditional = m_conditional;
                    ASSERT(conditional);
                    m_conditional = conditional->parent;
                    delete conditional;
                }*/

                /// Read one token using all the expansion and directive-handling logic
                Token readToken()
                {
                    // return m_expansionStream->readToken();
                }

                // Lexer* getLexer() { return m_lexerStream->getLexer(); }

                //ExpansionInputStream* getExpansionStream() { return m_expansionStream; }

            private:
                friend class Preprocessor;

                /// The parent preprocessor
                //Preprocessor* m_preprocessor = nullptr;

                /// The next outer input file
                ///
                /// E.g., if this file was `#include`d from another file, then `parent_` would be
                /// the file with the `#include` directive.
                ///
                std::shared_ptr<InputFile> parent_;

                /// The inner-most preprocessor conditional active for this file.
                //Conditional* m_conditional = nullptr;

                /// The lexer input stream that unexpanded tokens will be read from
                //  LexerInputStream* m_lexerStream;

                /// An input stream that applies macro expansion to `m_lexerStream`
                // ExpansionInputStream* m_expansionStream;
            };

            Preprocessor::~Preprocessor()
            {
            }

            Preprocessor::Preprocessor(const std::shared_ptr<SourceFile>& sourceFile, const std::shared_ptr<DiagnosticSink>& diagnosticSink)
                : sourceFile_(sourceFile),
                  sink_(diagnosticSink)
            {
                auto sourceView = std::make_shared<SourceView>(&*sourceFile_, nullptr);

                lexer_ = std::make_unique<Lexer>(sourceView, diagnosticSink);
            }

            void Preprocessor::pushInputFile(const std::shared_ptr<InputFile>& inputFile)
            {
                inputFile->parent_ = currentInputFile_;
                currentInputFile_ = inputFile;
            }

            void Preprocessor::popInputFile()
            {
            }

            std::shared_ptr<std::vector<Token>> Preprocessor::ReadAllTokens()
            {
                auto tokens = std::make_shared<std::vector<Token>>();
                for (;;)
                {
                    Token token = ReadToken();

                    switch (token.type)
                    {
                        default:
                            tokens->push_back(token);
                            break;

                        case TokenType::EndOfFile:
                            // Note: we include the EOF token in the list,
                            // since that is expected by the `TokenList` type.
                            tokens->push_back(token);
                            return tokens;

                        case TokenType::WhiteSpace:
                        case TokenType::NewLine:
                        case TokenType::LineComment:
                        case TokenType::BlockComment:
                        case TokenType::Invalid:
                            break;
                    }
                }
            }

            Token Preprocessor::ReadToken()
            {
                for (;;)
                {
                    /* auto inputFile = currentInputFile_;
                    if (!inputFile)
                        return preprocessor->endOfFileToken;

                    auto expansionStream = inputFile->getExpansionStream();

                    // Look at the next raw token in the input.
                    Token token = expansionStream->peekRawToken();     */
                    Token token = lexer_->GetNextToken();
                    if (token.type == TokenType::EndOfFile)
                    {
                        //  preprocessor->popInputFile();
                        //  continue;
                        return token;
                    }

                    // If we have a directive (`#` at start of line) then handle it
                    if ((token.type == TokenType::Pound) && (token.flags & TokenFlag::AtStartOfLine))
                    {
                        // Skip the `#`
                        expansionStream->readRawToken();

                        // Create a context for parsing the directive
                        PreprocessorDirectiveContext directiveContext;
                        directiveContext.m_preprocessor = preprocessor;
                        directiveContext.m_parseError = false;
                        directiveContext.m_haveDoneEndOfDirectiveChecks = false;
                        directiveContext.m_inputFile = inputFile;

                        // Parse and handle the directive
                        HandleDirective(&directiveContext);
                        continue;
                    }

                    // otherwise, if we are currently in a skipping mode, then skip tokens
                    if (inputFile->isSkipping())
                    {
                        expansionStream->readRawToken();
                        continue;
                    }

                    token = expansionStream->peekToken();
                    if (token.type == TokenType::EndOfFile)
                    {
                        preprocessor->popInputFile();
                        continue;
                    }

                    expansionStream->readToken();
                    return token;
                }
            }
        }
    }
}