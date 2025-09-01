#include "cxxopts.hpp"

#include "ShaderBuilder.hpp"

static const std::string VERSION = "0.1";

namespace RR
{
    int entryPoint(int argc, char** argv)
    {
        cxxopts::Options options("Shader compiler", "Shader compiler");

        // clang-format off
        options.add_options("Common")
            ("h,help", "Print help")
            ("v,version", "Print version")
            ("command", "Command to execute (build/compile)", cxxopts::value<std::string>())
            ("files", "Input files", cxxopts::value<std::vector<std::string>>())
            ("J,include", "Include pathes", cxxopts::value<std::vector<std::string>>());
        // clang-format on

        options.add_options("compile");

        options.add_options("build")("output", "Output file", cxxopts::value<std::string>());

        options.parse_positional({"command", "files"});
        options.positional_help("COMMAND FILES...");

        try
        {
            auto result = options.parse(argc, argv);
            if (result.count("help") || result.arguments().empty())
            {
                std::cout << options.help({"Common", "build", "compile"}) << std::endl;
                return 0;
            }

            if (result.count("version"))
            {
                std::cout << "Shader compiler version " << VERSION << std::endl;
                return 0;
            }

            if (result.count("command"))
            {
                if (result["command"].as<std::string>() == "build")
                {
                    LibraryBuildDesc desc;

                    if (!result.count("files"))
                    {
                        std::cerr << "No files provided" << std::endl;
                        return 1;
                    }
                    else if (result.count("files") > 1)
                    {
                        std::cerr << "Multiple input files provided" << std::endl;
                        return 1;
                    }
                    else
                    {
                        const auto files = result["files"].as<std::vector<std::string>>();
                        desc.inputFile = files[0];
                    }

                    if (!result.count("output"))
                    {
                        std::cerr << "No output file provided" << std::endl;
                        return 1;
                    }

                    if(result.count("include"))
                        desc.includePathes = result["include"].as<std::vector<std::string>>();

                    desc.outputFile = result["output"].as<std::string>();

                    if (ShaderBuilder::Instance().BuildLibrary(desc) != Common::RResult::Ok)
                    {
                        std::cerr << "Failed to build library" << std::endl;
                        return 1;
                    }
                }
                else if (result["command"].as<std::string>() == "compile")
                {
                    std::cout << "Compile shader" << std::endl;
                }
                else
                {
                    std::cerr << "Invalid command: " << result["command"].as<std::string>() << std::endl;
                    return 1;
                }
            }
        }
        catch (const cxxopts::OptionException& e)
        {
            std::cerr << "error parsing options: " << e.what() << std::endl;
            return 1;
        }

        return 0;
    }
}

#if defined(OS_WINDOWS) && defined(UNICODE)
#include <Windows.h>
#include <shellapi.h>
#include <wchar.h>
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPSTR lpCmdLine, int nCmdShow)
{
    std::ignore = hInst;
    std::ignore = hPreInst;
    std::ignore = lpCmdLine;
    std::ignore = nCmdShow;

    int argc;
    wchar_t** lpArgv = CommandLineToArgvW(GetCommandLineW(), &argc);
    char** argv = new char*[argc];

    for (int i = 0; i < argc; ++i)
    {
        const auto size = wcslen(lpArgv[i]) + 1;
        argv[i] = new char[size];
        size_t countConverted;

        wcsrtombs_s(
            &countConverted,
            argv[i],
            size,
            const_cast<const wchar_t**>(&lpArgv[i]),
            size - 1,
            nullptr);
    }
    LocalFree(lpArgv);

    const auto exitCode = RR::entryPoint(argc, argv);

    for (int i = 0; i < argc; ++i)
        delete[] (argv[i]);
    delete[] (argv);

    return exitCode;
}
#else
int main(int argc, char** argv)
{
    return RR::entryPoint(argc, argv);
}
#endif
