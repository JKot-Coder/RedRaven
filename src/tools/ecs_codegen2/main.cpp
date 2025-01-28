#include "Parser.hpp"

#include "parse_tools/core/FileSystem.hpp"
#include "parse_tools/core/IncludeSystem.hpp"
#include "parse_tools/core/CompileContext.hpp"
#include "parse_tools/core/SourceManager.hpp"
#include "parse_tools/Preprocessor.hpp"

#include "common/Result.hpp"

#include "stdio.h"
#include <fstream>
#include <iostream>
#include <filesystem>

static constexpr const char* VERSION_STR = "1.0";

struct OutputStream
{
    OutputStream(const std::string& outputPath) : stream(outputPath) { }
    std::ofstream stream;
    int32_t intendLen = 0;

    bool is_open() const { return stream.is_open(); }

    OutputStream& intend(int intendCnt = 1)
    {
        for (auto i = intendCnt * 4; i > 0; i--)
            stream.put(' ');

        return *this;
    }
    template<typename T>
    OutputStream& operator<<(const T& str)
    {
        stream << str;
        return *this;
    }

    OutputStream& operator<<(std::ostream& (*manip)(std::ostream&))
    {
        stream << manip;
        return *this;
    }

    template<typename T>
    OutputStream& operator<<(T&& value)
    {
        stream << std::forward<T>(value);
        return *this;
    }
};


struct EcsTypeInformation
{
    std::string filename;
    struct SystemDesc
    {
        std::vector<std::string> args;
        std::string name;
    };

    std::vector<SystemDesc> systems;
};

void codegen(const std::string& outputPath, const EcsTypeInformation& ecsTypeInformation)
{
    OutputStream codegen(outputPath);

    if (!codegen.is_open())
    {
        std::cerr << "Error: Could not open file " << outputPath << " for writing.\n";
        return;
    }

    codegen << "// Generated with ECS Codegen Version: " << VERSION_STR << std::endl;
    codegen << "#include \"" << ecsTypeInformation.filename << '\"' << std::endl;

    for (const auto& system : ecsTypeInformation.systems)
    {
        codegen << "void register(RR::Ecs::World& world)" << std::endl;
        codegen << "{" << std::endl;
        {
            codegen.intend() << system.name << '(';
            for (size_t i = 0; i < system.args.size(); i++)
            {
                codegen << system.args[i];
                if (i < system.args.size() - 1)
                    codegen << system.args[i] << ", ";
            }
            codegen << ");" << std::endl;
        }
        codegen << "}" << std::endl;
    }
}

namespace RR
{
    Common::RResult main(int argc, char** argv, const std::shared_ptr<ParseTools::BufferWriter>& sinkOutput)
    {
        ASSERT(sinkOutput);
        if (argc != 3)
        {
            std::cerr << "Usage: " << argv[0] << " <source-file>" << " <output-file>\n";
            return Common::RResult::InvalidArgument;
        }

        std::string sourcePath = argv[1];
        std::string outputPath = argv[2];

        try
        {
            auto context = std::make_shared<ParseTools::CompileContext>(true);
            context->sink.AddWriter(sinkOutput);

            auto sourceManager = std::make_shared<ParseTools::SourceManager>(context);
            const auto& fileSystem = std::make_shared<ParseTools::OSFileSystem>();
            const auto& includeSystem = std::make_shared<ParseTools::IncludeSystem>(fileSystem);
            ParseTools::Preprocessor preprocessor(includeSystem, sourceManager, context);

            ParseTools::PathInfo pathInfo;
            RR_RETURN_ON_FAIL(includeSystem->FindFile(sourcePath, "", pathInfo));

            std::shared_ptr<ParseTools::SourceFile> sourceFile;
            RR_RETURN_ON_FAIL(sourceManager->LoadFile(pathInfo, sourceFile));

            preprocessor.PushInputFile(sourceFile);
            const auto& tokens = preprocessor.ReadAllTokens();

            Parser parser(tokens, context);
            RR_RETURN_ON_FAIL(parser.Parse());

            EcsTypeInformation ecsInfo;
            // Todo remove include filesystem only for this
            ecsInfo.filename = std::filesystem::path(sourcePath).filename().u8string();
        }
        catch (const utf8::exception& e)
        {
            std::cerr << "UTF8 exception: " << e.what() << std::endl;
            return Common::RResult::Abort;
        }

        return Common::RResult::Ok;
    }
}

int main(int argc, char** argv)
{
    auto sinkOutput = std::make_shared<RR::ParseTools::BufferWriter>();
    RR::Common::RResult result = RR::main(argc, argv, sinkOutput);
    if (RR_FAILED(result))
    {
        std::cerr << sinkOutput->GetBuffer() << std::endl;
        return 1;
    }

    return 0;
}