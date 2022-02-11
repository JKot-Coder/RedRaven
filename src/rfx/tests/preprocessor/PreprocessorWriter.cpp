#include "PreprocessorWriter.hpp"

#include "compiler/Preprocessor.hpp"

#include "core/StringEscapeUtil.hpp"

namespace RR::Rfx
{
    namespace Tests
    {
        namespace
        {
            class SourceWriter
            {
            public:
                void Emit(const Token& token)
                {
                    U8String escapedToken;
                    StringEscapeUtil::AppendEscaped(StringEscapeUtil::Style::JSON, token.stringSlice, escapedToken);

                    if (token.type == Token::Type::RBrace)
                        dedent();

                    if (IsSet(token.flags, Token::Flags::AtStartOfLine))
                    {
                        if (!output_.empty()) // Skip new line at the wery begining
                            output_ += "\n" + indentString_;
                    }
                    else if (IsSet(token.flags, Token::Flags::AfterWhitespace))
                    {
                        output_ += " ";
                    }

                    output_.append(token.stringSlice.Begin(), token.stringSlice.End());

                    if (token.type == Token::Type::LBrace)
                        intend();
                }

                U8String GetOutput()
                {
                    return output_;
                }

            private:
                inline void intend()
                {
                    indentLevel_++;
                    updateIndentStiring();
                }

                inline void dedent()
                {
                    ASSERT(indentLevel_ > 0);

                    if (indentLevel_ == 0)
                        return;

                    indentLevel_--;
                    updateIndentStiring();
                }

                void updateIndentStiring()
                {
                    indentString_ = "";

                    for (uint32_t i = 0; i < indentLevel_; i++)
                        indentString_ += "    ";
                }

            private:
                uint32_t indentLevel_ = 0;
                U8String indentString_ = "";
                U8String output_;
            };

            void writeDiagnosticLog(std::ofstream& ofs, const std::shared_ptr<BufferWriter>& disagnosticBuffer)
            {
                ofs << "Diagnostic:[\n";

                U8String line;
                std::istringstream logStream(disagnosticBuffer->GetBuffer());

                while (std::getline(logStream, line, '\n'))
                    ofs << fmt::format("\t{}\n", line);

                ofs << "]";
            }

            void writeTokens(std::ofstream& ofs, const std::shared_ptr<Preprocessor>& preprocessor)
            {
                std::ignore = preprocessor;

                ofs << "Output:[\n";

                const auto& tokens = preprocessor->ReadAllTokens();
                SourceWriter writer;

                for (const auto& token : tokens)
                    writer.Emit(token);

                U8String line;
                std::istringstream logStream(writer.GetOutput());

                while (std::getline(logStream, line, '\n'))
                    ofs << fmt::format("\t{}\n", line);

                ofs << "]";
            }
        }

        void PreprocessorWriter::write(std::string path) const
        {
            std::ofstream ofs(path);
            ASSERT(ofs.is_open())

            writeTokens(ofs, preprocessor_);

            ofs << "\n";

            writeDiagnosticLog(ofs, disagnosticBuffer_);

            ofs.close();
        }
    }
}