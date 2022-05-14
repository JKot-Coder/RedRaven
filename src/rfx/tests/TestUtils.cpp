#include "TestUtils.hpp"

#include "RfxApprover.hpp"
#include "rfx.hpp"

#include "command.h"

#include "ApprovalTests/ApprovalTests.hpp"
#include <catch2/catch.hpp>
#include <fstream>

#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace RR::Rfx::Tests
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

        class CommandLineTestParser final
        {
        public:
            RfxResult Parse(const fs::path& path, std::vector<std::string>& outCommandLineArgumets);

        private:
            static constexpr char32_t kEOF = 0xFFFFFF;

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

            inline bool isWhiteSpace() const
            {
                auto ch = peek();
                return ch == ' ' || ch == '\t';
            }

            void skipBlockComment();
            void skipWhiteSpaces();

            void tryCommandLineArgumets(std::vector<std::string>& outCommandLineArgumets);

            RfxResult readAllFile(const fs::path& path);

        private:
            fs::path path_;
            std::string content_;
            std::string::const_iterator cursor_;
            std::string::const_iterator end_;
        };

        RfxResult CommandLineTestParser::Parse(const fs::path& path, std::vector<std::string>& outCommandLineArgumets)
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
                            case '/': advance(); tryCommandLineArgumets(outCommandLineArgumets); break;
                            case '*': skipBlockComment(); break;
                            default: advance(); continue;
                        } // clang-format on
                        continue;

                    default:
                        advance();
                        continue;
                }
                break;
            }

            return outCommandLineArgumets.empty() ? RfxResult::NotFound : RfxResult::Ok;
        }

        void CommandLineTestParser::skipBlockComment()
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

        void CommandLineTestParser::skipWhiteSpaces()
        {
            ASSERT(isWhiteSpace());

            while (isWhiteSpace())
                advance();
        }

        void CommandLineTestParser::tryCommandLineArgumets(std::vector<std::string>& outCommandLineArgumets)
        {
            skipWhiteSpaces();

            if (peek() != ':')
                return;

            advance(); // Skip ':';

            auto begin = cursor_;

            for (;;)
            {
                switch (peek())
                { // clang-format off
                    case '\n': case '\r': 
                    case ' ': case '\t':
                    case '-': case kEOF:
                        break; // clang-format on
                    default: advance(); continue;
                }
                break;
            }

            const auto& command = std::string(begin, cursor_);
            if (command != "RUN_RFXC")
                return;

            begin = cursor_;
            for (;;)
            {
                switch (peek())
                {
                    case kEOF:
                    case '\n':
                    case '\r': break;

                    default: advance(); continue;
                }
                break;
            }

            outCommandLineArgumets.push_back(std::string(begin, cursor_));
        }

        RfxResult CommandLineTestParser::readAllFile(const fs::path& path)
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

        void runLexerTestOnFile(const fs::path& testFile, const fs::path& testDirectory)
        {
            std::ignore = testFile;
            std::ignore = testDirectory;
        }

        void runCommandLineTestOnFile(const fs::path& testFile, const fs::path& testDirectory)
        {
            std::vector<std::string> commandLineArguments;
            CommandLineTestParser commandLineTestParser;
            REQUIRE(RFX_SUCCEEDED(commandLineTestParser.Parse(testFile, commandLineArguments)));
            REQUIRE(!commandLineArguments.empty());

            for (size_t index = 0; index < commandLineArguments.size(); index++)
            {
                auto& commandLine = commandLineArguments[index];
                commandLine = std::regex_replace(commandLine, std::regex("%INPUT%"), testFile.u8string());

                const auto& commandResult = raymii::Command::exec(fmt::format("rfxc {} 2>&1", commandLine));

                std::error_code ec;
                auto relativePath = fs::relative(testFile, testDirectory, ec);
                REQUIRE(!(bool)ec);

                // Remove extension from relativePath
                relativePath = relativePath.parent_path() / relativePath.stem();

                auto indexSuffix = fmt::format((commandLineArguments.size() > 1) ? "_{}" : "", index);
                auto namer = ApprovalTests::TemplatedCustomNamer::create(
                    "{TestSourceDirectory}/{ApprovalsSubdirectory}/" + relativePath.u8string() + indexSuffix + ".{ApprovedOrReceived}.{FileExtension}");

                RfxApprover::verify2(commandResult, ApprovalTests::Options().withNamer(namer));
            }
        }
    }

    void runTestOnFile(const fs::path& testFile, const fs::path& testDirectory, TestType type)
    {
        DYNAMIC_SECTION(testFile.filename().u8string())
        {
            switch (type)
            {
                case TestType::CommandLine: runCommandLineTestOnFile(testFile, testDirectory); break;
                case TestType::LexerTest: runLexerTestOnFile(testFile, testDirectory); break;
                default: FAIL("Unknown TestType"); break;
            }
            /*
            ComPtr<ICompiler> compiler;
            REQUIRE(RFX_SUCCEEDED(RR::Rfx::GetComplierInstance(compiler.put())));

            std::error_code ec;
            auto relativePath = fs::relative(testFile, testDirectory, ec);
            REQUIRE(!(bool)ec);

            std::vector<CompilerRequestDescription> compileRequests;

            CompilerRequestParser configurationParser;
            REQUIRE(RFX_SUCCEEDED(configurationParser.Parse(testFile, compileRequests)));
            REQUIRE(!compileRequests.empty());

            std::vector<ComPtr<ICompileResult>> compileResults;
            for (const auto& compileRequest : compileRequests)
            {
                ComPtr<ICompileResult> compileResult;
                REQUIRE(RFX_SUCCEEDED(compiler->Compile(compileRequest, compileResult.put())));

                compileResults.push_back(compileResult);
            }

            // Remove extension from relativePath
            relativePath = relativePath.parent_path() / relativePath.stem();

            auto namer = ApprovalTests::TemplatedCustomNamer::create(
                "{TestSourceDirectory}/{ApprovalsSubdirectory}/" + relativePath.u8string() + ".{ApprovedOrReceived}.{FileExtension}");
            RfxApprover::verify(compileResults, ApprovalTests::Options().withNamer(namer));
            */
        }
    }

    void runTestsInDirectory(const fs::path& directory, TestType type)
    {
        for (const auto& entry : fs::recursive_directory_iterator(directory))
        {
            if (entry.path().extension() != ".rfx")
                continue;

            runTestOnFile(entry, directory, type);
        }
    }
}