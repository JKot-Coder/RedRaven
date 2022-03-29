#include "PreprocessorWriter.hpp"

namespace RR::Rfx
{
    namespace Tests
    {
        namespace
        {
            void writeDiagnosticLog(std::ofstream& ofs, const ComPtr<IBlob>& diagnosticOutput)
            {
                ASSERT(diagnosticOutput);

                ofs << "Diagnostic:[\n";

                U8String line;
                std::istringstream logStream((char*)diagnosticOutput->GetBufferPointer());

                while (std::getline(logStream, line, '\n'))
                    ofs << fmt::format("\t{}\n", line);

                ofs << "]";
            }

            void writeTokens(std::ofstream& ofs, const ComPtr<IBlob>& preprocesorOutput)
            {
                ASSERT(preprocesorOutput);

                ofs << "Output:[\n";

                U8String line;
                std::istringstream logStream((char*)preprocesorOutput->GetBufferPointer());

                while (std::getline(logStream, line, '\n'))
                    ofs << fmt::format("\t{}\n", line);

                ofs << "]";
            }
        }

        void PreprocessorWriter::write(std::string path) const
        {
            std::ofstream ofs(path);
            ASSERT(ofs.is_open())

            writeTokens(ofs, preprocesorOutput_);

            ofs << "\n";

            writeDiagnosticLog(ofs, diagnosticOutput_);

            ofs.close();
        }
    }
}