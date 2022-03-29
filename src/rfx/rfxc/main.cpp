#include "rfx.hpp"

#include "cxxopts.hpp"
#include <fstream>
#include <ostream>

namespace RR
{
    namespace
    {
        template <typename S, typename... Args>
        inline void printMessageAndExit(int exitCode, const S& format, Args&&... args)
        {
            std::cerr << fmt::format(format, args...) << std::endl;
            exit(exitCode);
        }

        inline void printMessageAndExit(int exitCode, Rfx::RfxResult result)
        {
            Rfx::ComPtr<Rfx::IBlob> message;

            if (RFX_SUCCEEDED(Rfx::GetErrorMessage(result, message.put())))
                printMessageAndExit(1, "Unexpected error: {}", message->GetBufferPointer());

            exit(exitCode);
        }
    }

    void writeOutput(const std::string& filename, const Rfx::ComPtr<Rfx::IBlob>& output)
    {
        assert(output);

        std::ofstream fs(filename, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!fs.is_open())
            return;

        fs.write(static_cast<const char*>(output->GetBufferPointer()), output->GetBufferSize());
        fs.close();
    }

    int runApp(int argc, char** argv)
    {
        cxxopts::Options options("rfxc", "Shader compiler");

        std::vector<std::string> inputFiles;

        // options.show_positional_help();
        // clang-format off
        options.add_options("Common")
            ("h,help", "Display available options")
            ("version", "Display compiler version information")          
            ("inputs", "Inputs ", cxxopts::value<std::vector<std::string>>(inputFiles));
      /*      ("d,debug", "Enable debugging") // a bool parameter
            ("i,integer", "Int param", cxxopts::value<int>())
            ("f,file", "File name", cxxopts::value<std::string>())
            ("v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))*/
        // clang-format on

        options.add_options("Compilation")("Fp", "Output preprocessed input to the given file", cxxopts::value<std::string>(), "file");

        options.positional_help("<inputs>");

        cxxopts::ParseResult parseResult;

        try
        {
            std::vector<std::string> pos_names = { "inputs" };
            options.parse_positional(pos_names.begin(), pos_names.end());

            parseResult = options.parse(argc, argv);
        }
        catch (const cxxopts::OptionException& e)
        {
            printMessageAndExit(1, "unknown options: {}", e.what());
        }

        try
        {
            if (parseResult.count("help"))
                printMessageAndExit(0, options.help({ "", "Common", "Compilation" }));

            if (parseResult.count("version"))
                printMessageAndExit(0, "version: 1.0.0");

            const bool outputRequired = parseResult.count("Fp");
            const bool inputPresent = !inputFiles.empty();

            if (!inputPresent)
                printMessageAndExit(1, "rfxc failed: Required input file argument is missing. use --help to get more information.");

            if (!outputRequired)
                return 0;

            Rfx::CompilerRequestDescription compilerRequest;

            compilerRequest.inputFile = inputFiles.front().c_str();
            compilerRequest.outputPreprocessorResult = parseResult.count("Fp");

            Rfx::ComPtr<Rfx::ICompileResult> compileResult;

            Rfx::RfxResult result = Rfx::RfxResult::Ok;

            if (RFX_FAILED(result = Rfx::Compile(compilerRequest, compileResult.put())))
            {
                switch (result)
                {
                    case Rfx::RfxResult::Ok:
                        break;
                    case Rfx::RfxResult::NotFound:
                    case Rfx::RfxResult::CannotOpen:
                        printMessageAndExit(1, "Cannot open file: {}", compilerRequest.inputFile);
                        break;
                    default:
                        printMessageAndExit(1, result);
                        break;
                }
            }

            const auto outputsCount = compileResult->GetOutputsCount();
            for (size_t i = 0; i < outputsCount; i++)
            {
                Rfx::CompileOutputType outputType;
                Rfx::ComPtr<Rfx::IBlob> output;

                if (RFX_FAILED(result = compileResult->GetOutput(i, outputType, output.put())))
                    printMessageAndExit(1, result);

                switch (outputType)
                {
                    case RR::Rfx::CompileOutputType::Preprocesed:
                        writeOutput(parseResult["Fp"].as<std::string>(), output);
                        break;
                    default:
                        ASSERT_MSG(false, "Unknown output");
                        break;
                }
            }
        }
        catch (const cxxopts::OptionException& e)
        {
            printMessageAndExit(1, "error parsing options: {}", e.what());
        }

        return 0;
    }
}

int main(int argc, char** argv)
{
    return RR::runApp(argc, argv);
}