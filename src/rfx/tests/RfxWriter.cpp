#include "RfxWriter.hpp"

namespace RR::Rfx
{
    namespace Tests
    {
        namespace
        {
            void writeOutput(std::ofstream& ofs, const std::string& outputName, const ComPtr<IBlob>& output)
            {
                ASSERT(output);

                ofs << outputName << ":[\n";

                U8String line;
                std::istringstream logStream((char*)output->GetBufferPointer());

                while (std::getline(logStream, line, '\n'))
                    ofs << fmt::format("\t{}\n", line);

                ofs << "]\n";
            }
        }

        void RfxWriter::write(std::string path) const
        {
            std::ofstream ofs(path);
            ASSERT(ofs.is_open())

            for (const auto& compilerResult : compileResults_)
            {
                ComPtr<IBlob> output;
                Rfx::CompileOutputType outputType;

                auto result = compilerResult->GetOutput(0, outputType, output.put());
                if (RFX_SUCCEEDED(result))
                    writeOutput(ofs, "Output", output);

                output = nullptr;
                result = compilerResult->GetDiagnosticOutput(output.put());
                if (RFX_SUCCEEDED(result))
                    writeOutput(ofs, "Diagnostic", output);
            }

            ofs.close();
        }
    }
}