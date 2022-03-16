#include "rfx.hpp"

#include "cxxopts.hpp"
#include <tuple>

namespace
{
    template <typename S, typename... Args>
    inline void printMessageAndExit(int exitCode, const S& format, Args&&... args)
    {
        std::cout << fmt::format(format, args...) << std::endl;
        exit(exitCode);
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

        options.add_options("Compilation")
            ("Fp", "Output preprocessed input to the given file", cxxopts::value<std::string>(), "file");

        options.positional_help("<inputs>");

        cxxopts::ParseResult result;

        try
        {
            std::vector<std::string> pos_names = { "inputs" };
            options.parse_positional(pos_names.begin(), pos_names.end());

            result = options.parse(argc, argv);
        }
        catch (const cxxopts::OptionException& e)
        {
            printMessageAndExit(1, "unknown options: {}", e.what());
        }

        try
        {
            if (result.count("help"))
                printMessageAndExit(0, options.help({ "", "Common", "Compilation" }));

            if (result.count("version"))
                printMessageAndExit(0, "version: 1.0.0");

            const bool outputRequired = result.count("Fp");
            const bool inputPresent = !inputFiles.empty();

            if (!inputPresent)
                printMessageAndExit(1, "rfxc failed: Required input file argument is missing. use --help to get more information.");

            if (!outputRequired)
                return 0;

            RR::Rfx::CompilerRequestDescription compilerRequest;

            compilerRequest.outputPreprocessorResult = result.count("Fp");
            


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
    return runApp(argc, argv);
}