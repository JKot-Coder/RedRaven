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
                    output_.append(token.stringSlice.Begin(), token.stringSlice.End());
                }

                U8String GetOutput()
                {
                    return output_;
                }

            private:
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

                for (const auto& token : tokens)
                {
                    if (token.type == Token::Type::NewLine)
                    {
                        ofs << '\n';
                        continue;
                    }

                    // TODO copying strings...
                    ofs << token.GetContentString();
                }

                ofs << "]";
            }
        }

        void PreprocessorWriter::write(std::string path) const
        {
            std::ofstream ofs(path, std::ofstream::binary);
            ASSERT(ofs.is_open())

            writeTokens(ofs, preprocessor_);

            ofs << "\n";

            writeDiagnosticLog(ofs, disagnosticBuffer_);

            ofs.close();
        }
    }
}