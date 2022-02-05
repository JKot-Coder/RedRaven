#include "LexerWriter.hpp"

#include "core/StringEscapeUtil.hpp"

namespace RR::Rfx
{
    namespace Tests
    {
        namespace
        {

            void writeDiagnosticLog(std::ofstream& ofs, const std::shared_ptr<BufferWriter>& disagnosticBuffer)
            {
                ofs << "\tDiagnostic:[\n";

                U8String line;
                std::istringstream logStream(disagnosticBuffer->GetBuffer());

                while (std::getline(logStream, line, '\n'))
                {
                    U8String escapedLine;
                    StringEscapeUtil::AppendEscaped(StringEscapeUtil::Style::JSON, line, escapedLine);

                    ofs << fmt::format("\t\t\"{}\",\n", escapedLine);
                }

                ofs << "\t],";
            }

            void writeTokens(std::ofstream& ofs, const std::shared_ptr<Lexer>& lexer)
            {
                ofs << "\tTokens:[\n";

                while (true)
                {
                    const auto& token = lexer->ReadToken();

                    U8String escapedToken;
                    StringEscapeUtil::AppendEscaped(StringEscapeUtil::Style::JSON, token.stringSlice, escapedToken);

                    ofs << fmt::format("\t\t{{Type:\"{0}\", Content:\"{1}\", Line:{2}, Column:{3}}},\n",
                                       RR::Rfx::TokenTypeToString(token.type),
                                       escapedToken,
                                       token.humaneSourceLocation.line,
                                       token.humaneSourceLocation.column);

                    if (token.type == TokenType::EndOfFile)
                        break;
                }

                ofs << "\t],";
            }

        }

        void LexerWriter::write(std::string path) const
        {
            std::ofstream ofs(path);
            ASSERT(ofs.is_open())

            ofs << "{\n";

            writeTokens(ofs, lexer_);

            ofs << "\n";

            writeDiagnosticLog(ofs, disagnosticBuffer_);

            ofs << "\n}";

            ofs.close();
        }
    }
}