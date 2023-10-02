#include "rfx.hpp"

#include "common/ComPtr.hpp"
#include "common/Result.hpp"
#include "stl/enum_array.hpp"

#include "cxxopts.hpp"
#include <fstream>
#include <ostream>
#include <sstream>

namespace RR
{
    namespace
    {
        template <typename S, typename... Args>
        void printErrorMessage(const S& format, Args&&... args)
        {
            std::cerr << fmt::format(format, args...) << std::endl;
        }

        inline void printErrorMessage(Rfx::RfxResult result)
        {
            printErrorMessage("Unexpected error: {}", Common::GetErrorMessage(result));
        }
    }

    void writeOutput(const std::string& filename, Rfx::CompileOutputType outputType, const Common::ComPtr<Rfx::IBlob>& output)
    {
        assert(output);

        const auto outputString = static_cast<const char*>(output->GetBufferPointer());

        if (filename == "%STD_OUTPUT%")
        {
            switch (outputType)
            {
                case Rfx::CompileOutputType::Object:
                    std::cout.write(static_cast<const char*>(output->GetBufferPointer()), output->GetBufferSize());
                    return;
                case Rfx::CompileOutputType::Diagnostic: std::cout << "Diagnostic output:" << std::endl; break;
                case Rfx::CompileOutputType::Tokens: std::cout << "Tokens output:" << std::endl; break;
                case Rfx::CompileOutputType::Source: std::cout << "Source output:" << std::endl; break;
                case Rfx::CompileOutputType::Assembly: std::cout << "Assembly output:" << std::endl; break;
                default: ASSERT_MSG(false, "Unknown output");
            }

            auto stream = std::istringstream(outputString);
            U8String line;
            while (getline(stream, line))
                std::cout << "    " << line << "\n";

            return;
        }

        std::ofstream fs(filename, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!fs.is_open())
            return;

        fs.write(outputString, output->GetBufferSize());
        fs.close();
    }

    void writeResultToOutputs(const Common::ComPtr<Rfx::ICompileResult>& compileResult,
                              const stl::enum_array<U8String, RR::Rfx::CompileOutputType>& outputs)
    {
        if (!compileResult)
            return;

        const auto outputsCount = compileResult->GetOutputsCount();
        for (size_t i = 0; i < outputsCount; i++)
        {
            Rfx::CompileOutputType outputType;
            Common::ComPtr<Rfx::IBlob> output;

            Rfx::RfxResult result;
            if (RR_FAILED(result = compileResult->GetOutput(i, outputType, output.put())))
            {
                printErrorMessage(result);
                continue;
            }

            switch (outputType)
            {
                case RR::Rfx::CompileOutputType::Diagnostic:
                    writeOutput("%STD_OUTPUT%", outputType, output);
                    break;
                case RR::Rfx::CompileOutputType::Source:
                case RR::Rfx::CompileOutputType::Assembly:
                case RR::Rfx::CompileOutputType::Object:
                    writeOutput(outputs[outputType], outputType, output);
                    break;
                case RR::Rfx::CompileOutputType::Tokens:
                    writeOutput("%STD_OUTPUT%", outputType, output);
                    break;
                default:
                    ASSERT_MSG(false, "Unknown output");
                    break;
            }
        }
    }

    int runApp(int argc, char** argv)
    {
        cxxopts::Options options("rfxc", "Shader compiler");

        std::vector<std::string> inputFiles;
        std::vector<std::string> definitions;

        // options.show_positional_help();

        options.add_options("Common") // clang-format off
            ("h,help", "Display available options")
            ("version", "Display compiler version information")
            ("inputs", "Inputs", cxxopts::value<std::vector<std::string>>(inputFiles));

      /*      ("d,debug", "Enable debugging") // a bool parameter
            ("i,integer", "Int param", cxxopts::value<int>())
            ("f,file", "File name", cxxopts::value<std::string>())
            ("v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))*/
        // clang-format on

        options.add_options("Compilation") // clang-format off
            ("Fc", "Output assembly code listing file", cxxopts::value<std::string>(), "file")
            ("Fo", "Output object file", cxxopts::value<std::string>(), "file")
            ("D", "Define macro", cxxopts::value<std::vector<std::string>>(definitions))
            ("Rl", "Force the usage of only relative paths in the output and diagnostic messages"); // clang-format on

        options.add_options("Utility Options") // clang-format off
            ("L", "Lexer output to file (must be used alone)", cxxopts::value<std::string>(), "file")
            ("P", "Preprocessor output to file (must be used alone)", cxxopts::value<std::string>(), "file"); // clang-format on

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
            printErrorMessage("unknown options: {}", e.what());
            return 1;
        }

        try
        {
            if (parseResult.count("help"))
            {
                std::cout << options.help({ "", "Common", "Compilation", "Utility Options" }) << std::endl;
                return 0;
            }

            if (parseResult.count("version"))
            {
                std::cout << "version: 1.0.0" << std::endl;
                return 0;
            }

            if (inputFiles.empty())
            {
                printErrorMessage("rfxc failed: Required input file argument is missing. use --help to get more information.");
                return 1;
            }

            Rfx::CompileRequestDescription compileRequestDesc {};

            compileRequestDesc.inputFile = inputFiles.front().c_str();

            stl::enum_array<U8String, RR::Rfx::CompileOutputType> outputs;

            compileRequestDesc.compilerOptions.onlyRelativePaths = parseResult.count("Rl");

            if (parseResult.count("P"))
            {
                compileRequestDesc.outputStage = Rfx::CompileRequestDescription::OutputStage::Preprocessor;
                outputs[RR::Rfx::CompileOutputType::Source] = parseResult["P"].as<std::string>();
            }
            else if (parseResult.count("L"))
            {
                compileRequestDesc.outputStage = Rfx::CompileRequestDescription::OutputStage::Lexer;
                outputs[RR::Rfx::CompileOutputType::Source] = parseResult["L"].as<std::string>();
            }
            else if (parseResult.count("Fc") || parseResult.count("Fo"))
            {
                compileRequestDesc.outputStage = Rfx::CompileRequestDescription::OutputStage::Compiler;

                compileRequestDesc.compilerOptions.assemblyOutput = parseResult.count("Fc");
                compileRequestDesc.compilerOptions.objectOutput = parseResult.count("Fo");

                outputs[RR::Rfx::CompileOutputType::Assembly] = parseResult["Fc"].as<std::string>();
                outputs[RR::Rfx::CompileOutputType::Object] = parseResult["Fo"].as<std::string>();
            }

            Rfx::Utils::CStringAllocator<char> cstingAllocator;
            cstingAllocator.Allocate(definitions);

            compileRequestDesc.defines = (const char**)cstingAllocator.GetCStrings().data();
            compileRequestDesc.defineCount = cstingAllocator.GetCStrings().size();

            Common::ComPtr<Rfx::ICompileResult> compileResult;
            Common::RResult result = Common::RResult::Ok;

            Common::ComPtr<Rfx::ICompiler> compiler;
            if (RR_FAILED(result = Rfx::GetComplierInstance(compiler.put())))
            {
                printErrorMessage("Can't get compiler instance");
                return 1;
            }

            if (RR_FAILED(result = compiler->Compile(compileRequestDesc, compileResult.put())))
            {
                switch (result)
                {
                    case Common::RResult::NotFound: printErrorMessage("File not found: {}", compileRequestDesc.inputFile); break;
                    case Common::RResult::CannotOpen: printErrorMessage("Cannot open file: {}", compileRequestDesc.inputFile); break;
                    default: printErrorMessage(result);
                }

                writeResultToOutputs(compileResult, outputs);
                return 1;
            }

            writeResultToOutputs(compileResult, outputs);
        }
        catch (const cxxopts::OptionException& e)
        {
            printErrorMessage("error parsing options: {}", e.what());
            return 1;
        }

        return 0;
    }
}

int main(int argc, char** argv)
{
    return RR::runApp(argc, argv);
}