#include "cxxopts.hpp"
#include "AssetImporter.hpp"

namespace RR::AssetImporter
{
    template <typename S, typename... Args>
    void printErrorMessage(const S& format, Args&&... args)
    {
        std::cerr << fmt::format(format, args...) << std::endl;
    }

    int runApp(int argc, char** argv)
    {
        cxxopts::Options options("asset_processor", "Asset processor");

        std::vector<std::string> inputDirectories;
        std::vector<std::string> definitions;

        // options.show_positional_help();

        options.add_options("Common") // clang-format off
            ("h,help", "Display available options")
            ("version", "Display version information")          
            ("inputs", "Inputs", cxxopts::value<std::vector<std::string>>(inputDirectories));

      /*      ("d,debug", "Enable debugging") // a bool parameter
            ("i,integer", "Int param", cxxopts::value<int>())
            ("f,file", "File name", cxxopts::value<std::string>())
            ("v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))*/
            // clang-format on
            /*
                    options.add_options("Compilation") // clang-format off
                        ("Fc", "Output assembly code listing file", cxxopts::value<std::string>(), "file")
                        ("Fo", "Output object file", cxxopts::value<std::string>(), "file")
                        ("D", "Define macro", cxxopts::value<std::vector<std::string>>(definitions)); // clang-format on

                    options.add_options("Utility Options") // clang-format off
                        ("P", "Preprocess to file (must be used alone)", cxxopts::value<std::string>(), "file"); // clang-format on
            */

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
        
        AssetImporter::Instance().ProcessAsset("test.rfx");
       // AssetImporter importer;
        //importer

        try
        {
            if (parseResult.count("help"))
            {
                std::cout << options.help({ "", "Common" }) << std::endl;
                return 0;
            }

            if (parseResult.count("version"))
            {
                std::cout << "version: 1.0.0" << std::endl;
                return 0;
            }

            if (inputDirectories.empty())
            {
                printErrorMessage("failed: Required input directory argument is missing. use --help to get more information.");
                return 1;
            }

            for(auto input : inputDirectories)
            {
                
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
    return RR::AssetImporter::runApp(argc, argv);
}