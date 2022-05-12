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

        template <typename T>
        class CStringAllocator final
        {
        public:
            ~CStringAllocator()
            {
                for (const auto cstring : cstring_)
                    delete[] cstring;
            }

            void Allocate(const std::vector<std::basic_string<T>>& strings)
            {
                for (const auto& string : strings)
                    Allocate(string);
            }

            T* Allocate(const std::basic_string<T>& string)
            {
                const auto stringLength = string.length();

                auto cString = new T[stringLength + 1];
                string.copy(cString, stringLength);
                cString[stringLength] = '\0';

                cstring_.push_back(cString);
                return cString;
            }

            const std::vector<T*>& GetCStrings() const { return cstring_; }

        private:
            std::vector<T*> cstring_;
        };
    }

    void writeOutput(const std::string& filename, Rfx::CompileOutputType outputType, const Rfx::ComPtr<Rfx::IBlob>& output)
    {
        assert(output);

        const auto outputString = static_cast<const char*>(output->GetBufferPointer());

        if (filename == "%STD_OUTPUT%")
        {
            switch (outputType)
            {
                case RR::Rfx::CompileOutputType::Diagnostic:
                    std::cout << "Diagnostic output:" << std::endl;
                    break;
                case RR::Rfx::CompileOutputType::Tokens:
                    std::cout << "Tokens output:" << std::endl;
                    break;
                case RR::Rfx::CompileOutputType::Object:
                    std::cout.write(static_cast<const char*>(output->GetBufferPointer()), output->GetBufferSize());
                    return;
                case RR::Rfx::CompileOutputType::Source:
                    std::cout << "Preprocessor output:" << std::endl;
                    break;
                case RR::Rfx::CompileOutputType::Assembly:
                    std::cout << "Assembly output:" << std::endl;
                    break;
                default:
                    ASSERT_MSG(false, "Unknown output");
            }

            std::cout << outputString;
            return;
        }

        std::ofstream fs(filename, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!fs.is_open())
            return;

        fs.write(outputString, output->GetBufferSize());
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
            ("Fc", "Output assembly code listing file", cxxopts::value<std::string>(), "file")
            ("Fo", "Output object file", cxxopts::value<std::string>(), "file")
            ("D", "Define macro", cxxopts::value<std::vector<std::string>>(definitions)); // clang-format on

        options.add_options("Utility Options") // clang-format off
            ("P", "Preprocess to file (must be used alone)", cxxopts::value<std::string>(), "file"); // clang-format on

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

            if (parseResult.count("P"))
            {
                compileRequestDesc.outputStage = Rfx::CompileRequestDescription::OutputStage::Preprocessor;
            }
            else if (parseResult.count("Fc") || parseResult.count("Fo"))
            {
                compileRequestDesc.outputStage = Rfx::CompileRequestDescription::OutputStage::Compiler;

                compileRequestDesc.compilerOptions.assemblyOutput = parseResult.count("Fc");
                compileRequestDesc.compilerOptions.objectOutput = parseResult.count("Fo");
            }

            CStringAllocator<char> cstingAllocator;
            cstingAllocator.Allocate(definitions);

            compileRequestDesc.defines = (const char**)cstingAllocator.GetCStrings().data();
            compileRequestDesc.defineCount = cstingAllocator.GetCStrings().size();

            Rfx::ComPtr<Rfx::ICompileResult> compileResult;
            Rfx::RfxResult result = Rfx::RfxResult::Ok;

            Rfx::ComPtr<Rfx::ICompiler> compiler;
            if (RFX_FAILED(result = Rfx::GetComplierInstance(compiler.put())))
            {
                printErrorMessage("Unexpeted error");
                return 1;
            }

            if (RFX_FAILED(result = compiler->Compile(compileRequestDesc, compileResult.put())))
            {
                switch (result)
                {
                    case Rfx::RfxResult::Ok:
                        break;
                    case Rfx::RfxResult::NotFound:
                    case Rfx::RfxResult::CannotOpen:
                        printErrorMessage("Cannot open file: {}", compileRequestDesc.inputFile);
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
                    case RR::Rfx::CompileOutputType::Diagnostic:
                        writeOutput("%STD_OUTPUT%", outputType, output);
                        break;
                    case RR::Rfx::CompileOutputType::Source:
                        writeOutput(parseResult["P"].as<std::string>(), outputType, output);
                        break;
                    case RR::Rfx::CompileOutputType::Assembly:
                        writeOutput(parseResult["Fc"].as<std::string>(), outputType, output);
                        break;
                    case RR::Rfx::CompileOutputType::Object:
                        writeOutput(parseResult["Fo"].as<std::string>(), outputType, output);
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