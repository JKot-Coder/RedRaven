#include "rfx/include/rfx.hpp"
#include "rfx/core/IncludeSystem.hpp"
#include "rfx/core/FileSystem.hpp"
#include "rfx/core/SourceLocation.hpp"

#include "rfx/compiler/DiagnosticSink.hpp"
#include "rfx/compiler/Preprocessor.hpp"

#include "tests/preprocessor/PreprocessorApprover.hpp"

#include <catch2/catch.hpp>

#include <filesystem>
namespace fs = std::filesystem;

namespace RR
{
    namespace Rfx
    {
        namespace Tests
        {
            TEST_CASE("PreprocessorTests", "[Preprocessor]")
            {
                std::string path = "../src/rfx/tests/preprocessor";
                for (const auto& entry : fs::directory_iterator(path))
                {
                    if (entry.path().extension() != ".rfx")
                        continue;

                    DYNAMIC_SECTION(entry.path().stem().u8string())
                    {
                        RR::Rfx::PathInfo pathInfo;

                        const auto& fileSystem = std::make_shared<RR::Rfx::OSFileSystem>();
                        const auto& includeSystem = std::make_shared<RR::Rfx::IncludeSystem>(fileSystem);
                        includeSystem->FindFile(entry.path().u8string(), "", pathInfo);
                        std::shared_ptr<RR::Rfx::SourceFile> sourceFile;

                        if (RFX_FAILED(includeSystem->LoadFile(pathInfo, sourceFile)))
                            return;

                        auto diagnosticSink = std::make_shared<RR::Rfx::DiagnosticSink>();

                        auto bufferWriter = std::make_shared<RR::Rfx::BufferWriter>();
                        diagnosticSink->AddWriter(bufferWriter);

                        const auto& preprocessor = std::make_shared<RR::Rfx::Preprocessor>(includeSystem, diagnosticSink);
                        preprocessor->PushInputFile(sourceFile);

                        PreprocessorApprover::verify(preprocessor, bufferWriter);
                    }
                }
            }
        }
    }
}