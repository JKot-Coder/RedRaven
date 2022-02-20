#include "cxxopts.hpp"
#include <tuple>

namespace
{
    int runApp(int argc, char** argv)
    {
        cxxopts::Options options("rfxc", "Shader compiler");

        // clang-format off
        options.add_options()
            ("h,help", "Print help")
            ("d,debug", "Enable debugging") // a bool parameter
            ("i,integer", "Int param", cxxopts::value<int>())
            ("f,file", "File name", cxxopts::value<std::string>())
            ("v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"));
        // clang-format on

        cxxopts::ParseResult result;

        try
        {
            result = options.parse(argc, argv);
        }
        catch (const cxxopts::OptionException& e)
        {
            std::cout << "unknown options: " << e.what() << std::endl;
            exit(1);
        }

        try
        {
            if (result.count("help"))
            {
                std::cout << options.help({ "", "Group" }) << std::endl;
                exit(0);
            }
        }
        catch (const cxxopts::OptionException& e)
        {
            std::cout << "error parsing options: " << e.what() << std::endl;
            exit(1);
        }

        return 0;
    }
}

int main(int argc, char** argv)
{
    return runApp(argc, argv);
}