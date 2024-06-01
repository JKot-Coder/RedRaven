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
                    case Rfx::CompileOutputType::Diagnostic: return "Diagnostic";
                    default: ASSERT_MSG(false, "Unsupported CompileOutputType");
                } // clang-format on

                return "";
            }

            void writeOutput(std::ofstream& ofs, const std::string& outputName, const Common::ComPtr<IBlob>& output)
            {
                ASSERT(output);

                ofs << outputName << ":[\n";

                U8String line;
                std::istringstream logStream((char*)output->GetBufferPointer());

                while (std::getline(logStream, line, '\n'))
                    ofs << fmt::format("    {}\n", line);

                ofs << "]\n";
            }
        }

        void RfxWriter::write(std::string path) const
        {
            std::ofstream ofs(path);
            ASSERT(ofs.is_open());

            const auto& compileResult = compileResult_;

            for (size_t j = 0; j < compileResult->GetOutputsCount(); j++)
            {
                Common::ComPtr<IBlob> output;
                Rfx::CompileOutputType outputType;

                auto result = compileResult->GetOutput(j, outputType, output.put());
                if (RR_SUCCEEDED(result))
                    writeOutput(ofs, getOutputName(outputType), output);
            }

            ofs.close();
        }

        void RfxWriter2::write(std::string path) const
        {
            std::ofstream ofs(path);
            ASSERT(ofs.is_open());

            ofs << "exit_status:" << commandResult_.exitstatus << "\n";
            ofs << "output:\n"
                << commandResult_.output;
            ofs.close();
        }
    }
}