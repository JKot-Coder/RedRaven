#include "rfx.hpp"

#include "RfxApprover.hpp"

#include "common/LinearAllocator.hpp"

#include <catch2/catch.hpp>
#include <filesystem>

namespace fs = std::filesystem;

namespace RR::Rfx
{
    namespace Tests
    {
        TEST_CASE("LexerTests", "[Lexer]")
        {
            // todo replace with universal
            std::string path = "../src/rfx/tests/lexer";
            for (const auto& entry : fs::recursive_directory_iterator(path))
            {
                if (entry.path().extension() != ".rfx")
                    continue;

                DYNAMIC_SECTION(entry.path().stem().u8string())
                {
                    ComPtr<ICompileResult> compileResult;

                    const auto fileName = entry.path().u8string();

                    CompilerRequestDescription compileRequest;
                    compileRequest.inputFile = fileName.c_str();
                    compileRequest.lexerOutput = true;

                    auto result = RR::Rfx::Compile(compileRequest, compileResult.put());
                    REQUIRE(RFX_SUCCEEDED(result));

                    Rfx::ComPtr<Rfx::IBlob> lexerOutput;

                    const auto outputsCount = compileResult->GetOutputsCount();
                    for (size_t i = 0; i < outputsCount; i++)
                    {
                        Rfx::CompileOutputType outputType;
                        Rfx::ComPtr<Rfx::IBlob> output;

                        result = compileResult->GetOutput(i, outputType, output.put());
                        REQUIRE(RFX_SUCCEEDED(result));

                        switch (outputType)
                        {
                            case RR::Rfx::CompileOutputType::Lexer:
                                lexerOutput = output;
                                break;
                            default:
                                ASSERT_MSG(false, "Unknown output");
                                break;
                        }
                    }

                    Rfx::ComPtr<Rfx::IBlob> diagnosticOutput;
                    result = compileResult->GetDiagnosticOutput(diagnosticOutput.put());
                    REQUIRE(RFX_SUCCEEDED(result));

                    REQUIRE(lexerOutput);
                    REQUIRE(diagnosticOutput);

                    // Todo support relative
                    auto namer = ApprovalTests::TemplatedCustomNamer::create(
                        "{TestSourceDirectory}/{ApprovalsSubdirectory}/" + entry.path().stem().u8string() + ".{ApprovedOrReceived}.{FileExtension}");
                    RfxApprover::verify(lexerOutput, diagnosticOutput, ApprovalTests::Options().withNamer(namer));
                }
            }
        }
    }
}