#include "TestUtils.hpp"

#include "RfxApprover.hpp"
#include "rfx.hpp"

#include "command.h"

#include "common/Result.hpp"
#include "stl/enum.hpp"
#include "stl/enum_array.hpp"

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
#ifndef OS_WINDOWS
        constexpr char* ExabutablePrefix = ".\";
#else
        constexpr char* ExabutablePrefix = "";
#endif

        enum class TestCommand
        {
            RUN_RFXC,
            LEXER,
            PREPROCESSOR,
            Undefined
        };

        class CommandTestParser final
        {
        public:
            using CommandArgumentPair = std::pair<TestCommand, std::string>;

            RfxResult Parse(const fs::path& path, std::vector<CommandArgumentPair>& outCommandArgumentPairs);

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

            void tryParse(std::vector<CommandArgumentPair>& outCommandArgumentPairs);

            RfxResult readAllFile(const fs::path& path);

        private:
            fs::path path_;
            std::string content_;
            std::string::const_iterator cursor_;
            std::string::const_iterator end_;
        };

        Common::RResult CommandTestParser::Parse(const fs::path& path, std::vector<CommandArgumentPair>& outCommandArgumentPairs)
        {
            auto result = Common::RResult::Ok;

            if (RR_FAILED(result = readAllFile(path)))
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
                            case '/': advance(); tryParse(outCommandArgumentPairs); break;
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

            return outCommandArgumentPairs.empty() ? Common::RResult::NotFound : Common::RResult::Ok;
        }

        void CommandTestParser::skipBlockComment()
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

        void CommandTestParser::skipWhiteSpaces()
        {
            while (isWhiteSpace())
                advance();
        }

        void CommandTestParser::tryParse(std::vector<CommandArgumentPair>& outCommandArgumentPairs)
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

            const auto& commandName = std::string(begin, cursor_);

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

            const auto command = stl::enum_cast<TestCommand>(commandName).value_or(TestCommand::Undefined);

            outCommandArgumentPairs.emplace_back(command, std::string(begin, cursor_));
        }

        Common::RResult CommandTestParser::readAllFile(const fs::path& path)
        {
            if (!fs::exists(path))
                return Common::RResult::NotFound;

            std::ifstream file(path);

            if (!file)
                return Common::RResult::Fail;

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
                return Common::RResult::CannotOpen; // It's too large to fit in memory.
            }

            path_ = path;
            end_ = content_.end();
            cursor_ = content_.begin();
            return Common::RResult::Ok;
        }

        std::shared_ptr<ApprovalTests::TemplatedCustomNamer> getNamerForTest(const fs::path& testFile, const fs::path& testDirectory, size_t index, size_t testsCount)
        {
            std::error_code ec;
            auto relativePath = fs::relative(testFile, testDirectory, ec);
            REQUIRE(!(bool)ec);

            // Remove extension from relativePath
            relativePath = relativePath.parent_path() / relativePath.stem();

            auto indexSuffix = fmt::format((testsCount > 1) ? "_{}" : "", index);
            return ApprovalTests::TemplatedCustomNamer::create(
                "{TestSourceDirectory}/{ApprovalsSubdirectory}/" + relativePath.u8string() + indexSuffix + ".{ApprovedOrReceived}.{FileExtension}");
        }

        void runCommandLineTestOnFile(const fs::path& testFile, const fs::path& testDirectory)
        {
            std::vector<CommandTestParser::CommandArgumentPair> testCommands;
            CommandTestParser commandTestParser;
            REQUIRE(RR_SUCCEEDED(commandTestParser.Parse(testFile, testCommands)));
            REQUIRE(!testCommands.empty());

            Utils::CStringAllocator<char> cstringAllocator;

            for (size_t index = 0; index < testCommands.size(); index++)
            {
                auto& command = testCommands[index];

                switch (command.first)
                {
                    case TestCommand::RUN_RFXC:
                    {
                        const auto arguments = std::regex_replace(command.second, std::regex("%INPUT%"), testFile.u8string());
                        const auto& commandResult = raymii::Command::exec(fmt::format("{}rfxc {} 2>&1", ExabutablePrefix, arguments));
                        const auto namer = getNamerForTest(testFile, testDirectory, index, testCommands.size());
                        RfxApprover::verify(commandResult, ApprovalTests::Options().withNamer(namer));
                        break;
                    }
                    case TestCommand::LEXER:
                    {
                        CompileRequestDescription request;
                        request.outputStage = CompileRequestDescription::OutputStage::Lexer;
                        request.inputFile = cstringAllocator.Allocate(testFile.u8string());
                        request.compilerOptions.onlyRelativePaths = true;

                        Common::ComPtr<Rfx::ICompileResult> compileResult;
                        Common::ComPtr<Rfx::ICompiler> compiler;
                        REQUIRE(RR_SUCCEEDED(Rfx::GetComplierInstance(compiler.put())));
                        REQUIRE(RR_SUCCEEDED(compiler->Compile(request, compileResult.put())));

                        const auto namer = getNamerForTest(testFile, testDirectory, index, testCommands.size());

                        RfxApprover::verify(compileResult, ApprovalTests::Options().withNamer(namer));
                        break;
                    }
                    case TestCommand::PREPROCESSOR:
                    {
                        CompileRequestDescription request;
                        request.outputStage = CompileRequestDescription::OutputStage::Preprocessor;
                        request.inputFile = cstringAllocator.Allocate(testFile.u8string());
                        request.compilerOptions.onlyRelativePaths = true;

                        Common::ComPtr<Rfx::ICompileResult> compileResult;
                        Common::ComPtr<Rfx::ICompiler> compiler;
                        REQUIRE(RR_SUCCEEDED(Rfx::GetComplierInstance(compiler.put())));
                        REQUIRE(RR_SUCCEEDED(compiler->Compile(request, compileResult.put())));

                        const auto namer = getNamerForTest(testFile, testDirectory, index, testCommands.size());
                        RfxApprover::verify(compileResult, ApprovalTests::Options().withNamer(namer));
                        break;
                    }

                    default: REQUIRE(false);
                }
            }
        }
    }

    void runTestOnFile(const fs::path& testFile, const fs::path& testDirectory)
    {
        DYNAMIC_SECTION(testFile.filename().u8string())
        {
            runCommandLineTestOnFile(testFile, testDirectory);
        }
    }

    void runTestsInDirectory(const fs::path& directory)
    {
        for (const auto& entry : fs::recursive_directory_iterator(directory))
        {
            if (entry.path().extension() != ".rfx")
                continue;

            runTestOnFile(entry, directory);
        }
    }
}