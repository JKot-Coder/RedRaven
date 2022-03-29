#include "RfxApprover.hpp"

#include "rfx.hpp"

#include <catch2/catch.hpp>

#include <filesystem>
namespace fs = std::filesystem;

namespace RR::Rfx
{
    namespace Tests
    {
        TEST_CASE("PreprocessorTests", "[Preprocessor]")
        {
            fs::path path = "../src/rfx/tests/preprocessor";
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
                    compileRequest.preprocessorOutput = true;

                    auto result = RR::Rfx::Compile(compileRequest, compileResult.put());
                    REQUIRE(RFX_SUCCEEDED(result));

                    Rfx::ComPtr<Rfx::IBlob> preprocessorOutput;

                    const auto outputsCount = compileResult->GetOutputsCount();
                    for (size_t i = 0; i < outputsCount; i++)
                    {
                        Rfx::CompileOutputType outputType;
                        Rfx::ComPtr<Rfx::IBlob> output;

                        result = compileResult->GetOutput(i, outputType, output.put());
                        REQUIRE(RFX_SUCCEEDED(result));

                        switch (outputType)
                        {
                            case RR::Rfx::CompileOutputType::Preprocessor:
                                preprocessorOutput = output;
                                break;
                            default:
                                ASSERT_MSG(false, "Unknown output");
                                break;
                        }
                    }

                    Rfx::ComPtr<Rfx::IBlob> diagnosticOutput;
                    result = compileResult->GetDiagnosticOutput(diagnosticOutput.put());
                    REQUIRE(RFX_SUCCEEDED(result));

                    REQUIRE(preprocessorOutput.operator bool());
                    REQUIRE(diagnosticOutput.operator bool());

                    std::error_code ec;
                    auto relativePath = fs::relative(entry, path, ec);
                    REQUIRE(!(bool)ec);

                    // Remove extension from path 
                    relativePath = relativePath.parent_path() / relativePath.stem();

                    auto namer = ApprovalTests::TemplatedCustomNamer::create(
                        "{TestSourceDirectory}/{ApprovalsSubdirectory}/" + relativePath.u8string() + ".{ApprovedOrReceived}.{FileExtension}");
                    RfxApprover::verify(preprocessorOutput, diagnosticOutput, ApprovalTests::Options().withNamer(namer));
                }
            }
        }
    }
}