#include "LexerWriter.hpp"

#include "compiler/DiagnosticSink.hpp"

#include "core/StringEscapeUtil.hpp"

namespace RR::Rfx
{
    namespace Tests
    {
        namespace
        {

            void writeDiagnosticSink(std::ofstream& ofs, const std::shared_ptr<Lexer>& lexer)
            {
                ofs << "\tDiagnostic:[\n";

                auto sink = lexer->GetDiagnosticSink();

                sink->GetSourceLineMaxLength();

                /* while (true)
                {
                    const auto& token = lexer->GetNextToken();

                    U8String escapedToken;
                    StringEscapeUtil::AppendEscaped(StringEscapeUtil::Style::JSON, token.stringSlice, escapedToken);

                    ofs << fmt::format("\t\t{{Type:\"{0}\", Content:\"{1}\", Line:{2}, Column:{3}}}",
                                       RR::Rfx::TokenTypeToString(token.type),
                                       escapedToken,
                                       token.humaneSourceLocation.line,
                                       token.humaneSourceLocation.column);

                    if (token.type == TokenType::EndOfFile)
                    {
                        ofs << "\n";
                        break;
                    }

                    ofs << ",\n";
                }*/

                ofs << "\t]\n";
            }

            void writeTokens(std::ofstream& ofs, const std::shared_ptr<Lexer>& lexer)
            {
                ofs << "\tTokens:[\n";

                while (true)
                {
                    const auto& token = lexer->GetNextToken();

                    U8String escapedToken;
                    StringEscapeUtil::AppendEscaped(StringEscapeUtil::Style::JSON, token.stringSlice, escapedToken);

                    ofs << fmt::format("\t\t{{Type:\"{0}\", Content:\"{1}\", Line:{2}, Column:{3}}}",
                                       RR::Rfx::TokenTypeToString(token.type),
                                       escapedToken,
                                       token.humaneSourceLocation.line,
                                       token.humaneSourceLocation.column);

                    if (token.type == TokenType::EndOfFile)
                    {
                        ofs << "\n";
                        break;
                    }

                    ofs << ",\n";
                }

                ofs << "\t]\n";
            }

        }

        void LexerWriter::write(std::string path) const
        {
            std::ofstream ofs(path);
            ASSERT(ofs.is_open())

            ofs << "{\n";

            writeDiagnosticSink(ofs, tokenList_);
            writeTokens(ofs, lexer_);

            ofs << "}\n";

            ofs.close();
        }
    }
}