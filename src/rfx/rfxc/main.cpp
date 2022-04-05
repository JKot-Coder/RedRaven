#include "rfx.hpp"

#include "cxxopts.hpp"
#include <fstream>
#include <ostream>

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
            Rfx::ComPtr<Rfx::IBlob> message;

            if (RFX_SUCCEEDED(Rfx::GetErrorMessage(result, message.put())))
                printErrorMessage("Unexpected error: {}", message->GetBufferPointer());
        }

        class DefinitionsParser final
        {
        public:
            ~DefinitionsParser()
            {
                for (const auto cString : cStrings_)
                    delete[] cString;
            }

            const std::vector<Rfx::CompilerRequestDescription::PreprocessorDefinition>& Parse(const std::vector<std::string>& definitions)
            {
                for (const auto& define : definitions)
                {
                    Rfx::CompilerRequestDescription::PreprocessorDefinition preprocDefine;
                    const auto delimiterPos = define.find('=');

                    if (delimiterPos == std::string::npos)
                    {
                        preprocDefine.key = allocateCString(define);
                        preprocDefine.value = nullptr;
                    }
                    else
                    {
                        preprocDefine.key = allocateCString(define.substr(0, delimiterPos));
                        preprocDefine.value = allocateCString(define.substr(delimiterPos + 1, define.length()));
                    }

                    definitions_.push_back(preprocDefine);
                }

                return definitions_;
            }

        private:
            char* allocateCString(const std::string& string)
            {
                const auto stringLength = string.length();

                auto cString = new char[stringLength + 1];
                string.copy(cString, stringLength);
                cString[stringLength] = '\0';

                cStrings_.push_back(cString);
                return cString;
            }

        private:
            std::vector<Rfx::CompilerRequestDescription::PreprocessorDefinition> definitions_;
            std::vector<char*> cStrings_;
        };
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
            ("Fp", "Output preprocessed input to the given file", cxxopts::value<std::string>(), "file")
            ("D", "Define macro", cxxopts::value<std::vector<std::string>>(definitions));
        // clang-format on

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
                std::cout << options.help({ "", "Common", "Compilation" }) << std::endl;
                return 0;
            }

            if (parseResult.count("version"))
            {
                std::cout << "version: 1.0.0" << std::endl;
                return 0;
            }

            const bool outputRequired = parseResult.count("Fp");
            const bool inputPresent = !inputFiles.empty();

            if (!inputPresent)
            {
                printErrorMessage("rfxc failed: Required input file argument is missing. use --help to get more information.");
                return 1;
            }

            if (!outputRequired)
                return 0;

            Rfx::CompilerRequestDescription compilerRequest;

            compilerRequest.inputFile = inputFiles.front().c_str();
            compilerRequest.preprocessorOutput = parseResult.count("Fp");

            DefinitionsParser definitionsParser;
            const auto preprocessorDefinitions = definitionsParser.Parse(definitions);

            compilerRequest.defines = preprocessorDefinitions.data();
            compilerRequest.defineCount = preprocessorDefinitions.size();

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
                        printErrorMessage("Cannot open file: {}", compilerRequest.inputFile);
                        return 1;
                    default:
                        printErrorMessage(result);
                        return 1;
                }
            }

            const auto outputsCount = compileResult->GetOutputsCount();
            for (size_t i = 0; i < outputsCount; i++)
            {
                Rfx::CompileOutputType outputType;
                Rfx::ComPtr<Rfx::IBlob> output;

                if (RFX_FAILED(result = compileResult->GetOutput(i, outputType, output.put())))
                {
                    printErrorMessage(result);
                    return 1;
                }

                switch (outputType)
                {
                    case RR::Rfx::CompileOutputType::Preprocessor:
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