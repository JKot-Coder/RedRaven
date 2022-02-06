#include "rfx/core/FileSystem.hpp"
#include "rfx/core/IncludeSystem.hpp"
#include "rfx/core/SourceLocation.hpp"
#include "rfx/include/rfx.hpp"

#include "rfx/compiler/DiagnosticSink.hpp"
#include "rfx/compiler/Lexer.hpp"

#include "tests/lexer/LexerApprover.hpp"

#include <catch2/catch.hpp>

#include <filesystem>
namespace fs = std::filesystem;

namespace RR
{
    namespace Rfx
    {
        namespace Tests
        {
            TEST_CASE("LexerTests", "[Lexer]")
            {
                std::string testsPath = "../src/rfx/tests";
                std::string lexerTestsPath = "../src/rfx/tests/Lexer";

                for (const auto& entry : fs::directory_iterator(lexerTestsPath))
                {
                    if (entry.path().extension() != ".rfx")
                        continue;

                    const auto& testFilename = entry.path().stem().u8string();
                    DYNAMIC_SECTION(testFilename)
                    {
                        std::error_code ec;
                        const auto& testPath = fs::relative(entry.path().u8string(),
                                                            testsPath,
                                                            ec).u8string();
                        if (ec)
                            return;

                        RR::Rfx::PathInfo pathInfo;

                        const auto& fileSystem = std::make_shared<RR::Rfx::OSFileSystem>();
                        const auto& includeSystem = std::make_shared<RR::Rfx::IncludeSystem>(fileSystem);
                        includeSystem->FindFile(testPath, testsPath, pathInfo);
                        std::shared_ptr<RR::Rfx::SourceFile> sourceFile;

                        if (RFX_FAILED(includeSystem->LoadFile(pathInfo, sourceFile)))
                            return;

                        const auto& sourceView = SourceView::Create(sourceFile);

                        auto diagnosticSink = std::make_shared<RR::Rfx::DiagnosticSink>();

                        auto bufferWriter = std::make_shared<RR::Rfx::BufferWriter>();
                        diagnosticSink->AddWriter(bufferWriter);

                        const auto& lexer = std::make_shared<RR::Rfx::Lexer>(sourceView, diagnosticSink);

                        LexerApprover::verify(lexer, bufferWriter);
                    }
                }
            }
        }
    }
}