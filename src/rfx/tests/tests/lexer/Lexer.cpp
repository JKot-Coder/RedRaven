#include "rfx/include/rfx.hpp"
#include "rfx/core/IncludeSystem.hpp"
#include "rfx/core/FileSystem.hpp"
#include "rfx/core/SourceLocation.hpp"

#include "rfx/compiler/DiagnosticSink.hpp"
#include "rfx/compiler/Lexer.hpp"

#include "tests/tests/lexer/LexerApprover.hpp"

#include <catch2/catch.hpp>

#include <filesystem>
namespace fs = std::filesystem;

namespace RR
{
    namespace Rfx
    {
        namespace Tests
        {
            TEST_CASE("Lexer", "[Lexer]")
            {
                std::string path = "../src/rfx/tests/tests/lexer";
                for (const auto& entry : fs::directory_iterator(path))
                {
                    if (entry.path().extension() != ".rfx")
                        continue;

                    DYNAMIC_SECTION(fmt::format("[Lexer::{}]", entry.path().u8string()))
                    {
                        RR::Rfx::PathInfo pathInfo;

                        const auto& fileSystem = std::make_shared<RR::Rfx::OSFileSystem>();
                        const auto& includeSystem = std::make_shared<RR::Rfx::IncludeSystem>(fileSystem);
                        includeSystem->FindFile(entry.path().u8string(), "", pathInfo);
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