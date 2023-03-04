#include "RfxWriter.hpp"

#include "command.h"

#include "common/Result.hpp"

namespace RR::Rfx
{
    namespace Tests
    {
        namespace
        {
            std::string getOutputName(Rfx::CompileOutputType outputType)
            {
                switch (outputType)
                { // clang-format off
                    case Rfx::CompileOutputType::Tokens: return "TokensOutput";
                    case Rfx::CompileOutputType::Source: return "SourceOutput";
                    default: ASSERT_MSG(false, "Unsupported CompileOutputType"); 
                } // clang-format on

                return "";
            }

            void writeOutput(std::ofstream& ofs, const std::string& outputName, const std::string& indent, const Common::ComPtr<IBlob>& output)
            {
                ASSERT(output);

                ofs << indent << outputName << ":[\n";

                U8String line;
                std::istringstream logStream((char*)output->GetBufferPointer());

                while (std::getline(logStream, line, '\n'))
                    ofs << indent << fmt::format("\t{}\n", line);

                ofs << indent << "]\n";
            }
        }

        void RfxWriter::write(std::string path) const
        {
            std::ofstream ofs(path);
            ASSERT(ofs.is_open())

            const size_t resultsCount = compileResults_.size();
            const std::string intent = (resultsCount > 1) ? "\t" : "";

            for (size_t i = 0; i < resultsCount; i++)
            {
                const auto& compileResult = compileResults_[i];

                if (resultsCount > 1)
                    ofs << "Test_" << (i + 1) << ":[\n";

                for (size_t j = 0; j < compileResult->GetOutputsCount(); j++)
                {
                    Common::ComPtr<IBlob> output;
                    Rfx::CompileOutputType outputType;

                    auto result = compileResult->GetOutput(j, outputType, output.put());
                    if (RR_SUCCEEDED(result))
                        writeOutput(ofs, getOutputName(outputType), intent, output);
                }

             //   ComPtr<IBlob> output = nullptr;
           //     auto result = compileResult->GetDiagnosticOutput(output.put());
           //     if (RFX_SUCCEEDED(result))
            //        writeOutput(ofs, "Diagnostic", intent, output);

                if (resultsCount > 1)
                    ofs << "]" << ((i != resultsCount - 1) ? "," : "") << "\n";
            }

            ofs.close();
        }

        void RfxWriter2::write(std::string path) const
        {
            std::ofstream ofs(path);
            ASSERT(ofs.is_open())

            ofs << "exit_status:" << commandResult_.exitstatus << "\n";
            ofs << "output: [\n" << commandResult_.output << "]\n";
            ofs.close();
        }
    }
}