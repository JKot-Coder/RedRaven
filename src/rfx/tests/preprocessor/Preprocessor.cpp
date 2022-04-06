#include "RfxApprover.hpp"

#include "rfx.hpp"

#include <catch2/catch.hpp>

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace RR::Rfx
{
    namespace Tests
    {
        namespace
        {
            U8String readAllFile(std::ifstream& file)
            {
                ASSERT(file);

                file.seekg(0, file.end);
                uint64_t sizeInBytes = file.tellg();
                file.seekg(0, file.beg);

                const uint64_t MaxFileSize = 0x40000000; // 1 Gib
                if (sizeInBytes > MaxFileSize)
                    return ""; // It's too large to fit in memory.

                U8String content;
                content.resize(sizeInBytes);

                file.read(&content[0], content.size());

                return content;
            }

            class CompilerRequestParser final
            {
            public:
                ~CompilerRequestParser()
                {
                    for (auto cstring : cstrings_)
                        delete[] cstring;
                }

                RfxResult Parse(const fs::path& path, std::vector<Rfx::CompilerRequestDescription>& outCompilerRequests);

            private:
                static constexpr char32_t kEOF = 0xFFFFFF;

            private:
                char* allocateCString(const std::string& string)
                {
                    const auto stringLength = string.length();

                    auto cString = new char[stringLength + 1];
                    string.copy(cString, stringLength);
                    cString[stringLength] = '\0';

                    cstrings_.push_back(cString);
                    return cString;
                }

                inline bool isReachEOF() const { return cursor_ == end_; }

                inline void skipBOM()
                {
                    auto it = cursor_;
                    if (((it != end_) && uint8_t(*it++) == 0xef) &&
                        ((it != end_) && uint8_t(*it++) == 0xbb) &&
                        ((it != end_) && uint8_t(*it++) == 0xbf))
                        cursor_ = it;
                }

                inline void advance()
                {
                    ASSERT(!isReachEOF());
                    cursor_++;
                }

                inline char32_t peek() const
                {
                    if (isReachEOF())
                        return kEOF;

                    return *cursor_;
                }

                void handleBlockComment();
                void tryParseCompilerRequest(std::vector<Rfx::CompilerRequestDescription>& outCompilerRequests);
                void tryParseCompilerRequest(std::string commandLine, std::vector<Rfx::CompilerRequestDescription>& outCompilerRequests);
                RfxResult readAllFile(const fs::path& path);

            private:
                fs::path path_;
                std::string content_;
                std::string::const_iterator cursor_;
                std::string::const_iterator end_;
                std::vector<char*> cstrings_;
            };

            void CompilerRequestParser::tryParseCompilerRequest(std::vector<Rfx::CompilerRequestDescription>& outCompilerRequests)
            {
                const auto commandBegin = cursor_;

                for (;;)
                {
                    switch (peek())
                    { // clang-format off
                        case kEOF: case '\n': case '\r':
                            tryParseCompilerRequest(std::string(commandBegin, cursor_), outCompilerRequests);
                            return;
                        default: advance(); continue;
                    } // clang-format on
                }
            }

            void CompilerRequestParser::tryParseCompilerRequest(std::string commandLine, std::vector<Rfx::CompilerRequestDescription>& outCompilerRequests)
            {
                Rfx::CompilerRequestDescription compilerRequest;

                for (;;)
                {
                    const auto delimiter = commandLine.find(':');

                    if (delimiter == std::string::npos)
                        break;

                    const auto command = commandLine.substr(0, delimiter);

                    compilerRequest.preprocessorOutput = (command == "PREPROCESSOR_TEST");
                    compilerRequest.lexerOutput = (command == "LEXER_TEST");

                    commandLine = commandLine.substr(delimiter + 1, commandLine.length());
                }

                if (compilerRequest.preprocessorOutput || compilerRequest.lexerOutput)
                {
                    compilerRequest.inputFile = allocateCString(path_.u8string());
                    outCompilerRequests.push_back(compilerRequest);
                }
            }

            void CompilerRequestParser::handleBlockComment()
            {
                ASSERT(peek() == '*')

                for (;;)
                {
                    switch (peek())
                    { // clang-format off
                        case kEOF: return;
                        case '*':
                            advance();
                            switch (peek())
                            {
                                case '/': advance(); return;
                                default: continue;
                            }
                        default: advance(); continue;
                    } // clang-format on
                }
            }

            RfxResult CompilerRequestParser::readAllFile(const fs::path& path)
            {
                if (!fs::exists(path))
                    return RfxResult::NotFound;

                std::ifstream file(path);

                if (!file)
                    return RfxResult::Fail;

                file.seekg(0, file.end);
                uint64_t sizeInBytes = file.tellg();
                file.seekg(0, file.beg);

                const uint64_t MaxFileSize = 0x40000000; // 1 Gib
                if (sizeInBytes < MaxFileSize)
                {
                    content_.resize(sizeInBytes);
                    file.read(&content_[0], content_.size());
                    file.close();
                }
                else
                {
                    file.close();
                    return RfxResult::CannotOpen; // It's too large to fit in memory.
                }

                path_ = path;
                end_ = content_.end();
                cursor_ = content_.begin();
                return RfxResult::Ok;
            }

            RfxResult CompilerRequestParser::Parse(const fs::path& path, std::vector<Rfx::CompilerRequestDescription>& outCompilerRequests)
            {
                auto result = RfxResult::Ok;

                if (RFX_FAILED(result = readAllFile(path)))
                    return result;

                skipBOM();

                while (!isReachEOF())
                {
                    switch (peek())
                    {
                        case '/':
                            advance();
                            switch (peek())
                            { // clang-format off
                                case '/': advance(); tryParseCompilerRequest(outCompilerRequests); break;
                                case '*': handleBlockComment(); break;
                                default: advance(); continue;
                            } // clang-format on
                            continue;

                        default:
                            advance();
                            continue;
                    }
                    break;
                }

                return outCompilerRequests.empty() ? RfxResult::NotFound : RfxResult::Ok;
            }
        }

        TEST_CASE("PreprocessorTests", "[Preprocessor]")
        {
            fs::path path = "../src/rfx/tests/preprocessor";
            for (const auto& entry : fs::recursive_directory_iterator(path))
            {
                if (entry.path().extension() != ".rfx")
                    continue;

                DYNAMIC_SECTION(entry.path().stem().u8string())
                {
                    const auto fileName = entry.path().u8string();

                    std::vector<CompilerRequestDescription> compileRequests;

                    CompilerRequestParser configurationParser;
                    auto result = configurationParser.Parse(entry, compileRequests);
                    REQUIRE(RFX_SUCCEEDED(result));
                    REQUIRE(!compileRequests.empty());

                    std::vector<ComPtr<ICompileResult>> compileResults;
                    for (const auto& compileRequest : compileRequests)
                    {
                        ComPtr<ICompileResult> compileResult;

                        result = RR::Rfx::Compile(compileRequest, compileResult.put());
                        REQUIRE(RFX_SUCCEEDED(result));

                        compileResults.push_back(compileResult);

                        /*
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
                        REQUIRE(diagnosticOutput.operator bool());   */
                    }

                    std::error_code ec;
                    auto relativePath = fs::relative(entry, path, ec);
                    REQUIRE(!(bool)ec);

                    // Remove extension from path
                    relativePath = relativePath.parent_path() / relativePath.stem();

                    auto namer = ApprovalTests::TemplatedCustomNamer::create(
                        "{TestSourceDirectory}/{ApprovalsSubdirectory}/" + relativePath.u8string() + ".{ApprovedOrReceived}.{FileExtension}");
                    RfxApprover::verify(compileResults, ApprovalTests::Options().withNamer(namer));
                }
            }
        }
    }
}