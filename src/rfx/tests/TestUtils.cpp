#include "TestUtils.hpp"

#include "RfxApprover.hpp"
#include "rfx.hpp"

#include "ApprovalTests/ApprovalTests.hpp"
#include <catch2/catch.hpp>
#include <fstream>

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

        class CompilerRequestParser final
        {
        public:
            using PreprocessorDefinition = Rfx::CompilerRequestDescription::PreprocessorDefinition;

            ~CompilerRequestParser()
            {
                for (auto cstring : cstrings_)
                    delete[] cstring;

                for (auto definition : definitionsArrays_)
                    delete[] definition;
            }

            RfxResult Parse(const fs::path& path, std::vector<Rfx::CompilerRequestDescription>& outCompilerRequests);

        private:
            static constexpr char32_t kEOF = 0xFFFFFF;

            struct Argument
            {
                enum class Type : uint32_t
                {
                    Unknown,
                    Define,
                    Output,
                };

                Type type = Type::Unknown;
                U8String value;
            };

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

            PreprocessorDefinition* allocateDefinitionsArray(const std::vector<PreprocessorDefinition>& definitions)
            {
                if (definitions.size() == 0)
                    return nullptr;

                const auto definitonsArray = new PreprocessorDefinition[definitions.size()];

                for (size_t index = 0; index < definitions.size(); index++)
                    definitonsArray[index] = definitions[index];

                definitionsArrays_.push_back(definitonsArray);
                return definitonsArray;
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

            inline bool isWhiteSpace() const
            {
                auto ch = peek();
                return ch == ' ' || ch == '\t';
            }

            void skipBlockComment();
            void skipWhiteSpaces();

            bool handleArgument(const Argument& argument, std::vector<PreprocessorDefinition>& definitions, Rfx::CompilerRequestDescription& outCompilerRequest);
            void tryParseCompilerRequest(std::vector<Rfx::CompilerRequestDescription>& outCompilerRequests);

            RfxResult readAllFile(const fs::path& path);

        private:
            fs::path path_;
            std::string content_;
            std::string::const_iterator cursor_;
            std::string::const_iterator end_;
            std::vector<char*> cstrings_;
            std::vector<PreprocessorDefinition*> definitionsArrays_;
        };

        bool CompilerRequestParser::handleArgument(const Argument& argument, std::vector<PreprocessorDefinition>& definitions, Rfx::CompilerRequestDescription& outCompilerRequest)
        {
            switch (argument.type)
            {
                case Argument::Type::Define:
                {
                    const auto delimiter = argument.value.find('=');

                    if (delimiter != std::string::npos)
                    {
                        definitions.push_back(
                            { allocateCString(argument.value.substr(0, delimiter)), // key
                              allocateCString(argument.value.substr(delimiter + 1, argument.value.size() - delimiter)) } // value
                        );
                    }
                    else
                    {
                        definitions.push_back({ allocateCString(argument.value), allocateCString("") });
                    }

                    return true;
                }

                case Argument::Type::Output:
                {
                    if (argument.value == "PREPROCESSOR")
                    {
                        outCompilerRequest.preprocessorOutput = true;
                    }
                    else if (argument.value == "LEXER")
                    {
                        outCompilerRequest.lexerOutput = true;
                    }
                    else
                    {
                        ASSERT_MSG(false, "Unknown output");
                        break;
                    }

                    return true;
                }

                default:
                    ASSERT_MSG(false, "Unknown argument type");
            }

            return false;
        }

        void CompilerRequestParser::tryParseCompilerRequest(std::vector<Rfx::CompilerRequestDescription>& outCompilerRequests)
        {
            bool validRequest = false;

            Rfx::CompilerRequestDescription compilerRequest {};
            std::vector<PreprocessorDefinition> definitions {};

            for (;;)
            {
                switch (peek())
                {
                    case kEOF:
                    case '\n':
                    case '\r':
                        if (!validRequest)
                            return;

                        compilerRequest.inputFile = allocateCString(path_.u8string());
                        compilerRequest.defines = allocateDefinitionsArray(definitions);
                        compilerRequest.defineCount = definitions.size();

                        outCompilerRequests.push_back(compilerRequest);
                        return;

                    case '-':
                    {
                        advance(); // Skip '-'

                        Argument argument = {};
                        switch (peek())
                        {
                            case 'O':
                                argument.type = Argument::Type::Output;
                                break;
                            case 'D':
                                argument.type = Argument::Type::Define;
                                break;
                            default:
                                return;
                        }

                        advance(); // skip argument type

                        if (!isWhiteSpace())
                            return;

                        skipWhiteSpaces();

                        auto argumentBegin = cursor_;

                        for (;;)
                        {
                            switch (peek())
                            { // clang-format off
                                case '\n': case '\r':
                                case ' ':  case '\t':
                                case '-':  case kEOF:
                                    break;
                                default:
                                    advance();
                                    continue;
                            } // clang-format on

                            break;
                        }

                        argument.value = std::string(argumentBegin, cursor_);

                        if (!handleArgument(argument, definitions, compilerRequest))
                            return;

                        if (argument.type == Argument::Type::Output)
                            validRequest = true;
                    }
                    break;

                    case '\t':
                    case ' ':
                        advance();
                        break;

                    default:
                        return;
                }
            }
        }

        void CompilerRequestParser::skipBlockComment()
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

        void CompilerRequestParser::skipWhiteSpaces()
        {
            ASSERT(isWhiteSpace());

            while (isWhiteSpace())
                advance();
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

            return outCompilerRequests.empty() ? RfxResult::NotFound : RfxResult::Ok;
        }
    }

    void runTestOnFile(const fs::path& testFile, const fs::path& testDirectory)
    {
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
            REQUIRE(RFX_SUCCEEDED(RR::Rfx::Compile(compileRequest, compileResult.put())));

            compileResults.push_back(compileResult);
        }

        // Remove extension from relativePath
        relativePath = relativePath.parent_path() / relativePath.stem();

        auto namer = ApprovalTests::TemplatedCustomNamer::create(
            "{TestSourceDirectory}/{ApprovalsSubdirectory}/" + relativePath.u8string() + ".{ApprovedOrReceived}.{FileExtension}");
        RfxApprover::verify(compileResults, ApprovalTests::Options().withNamer(namer));
    }

    void runTestsInDirectory(const fs::path& directory)
    {
        for (const auto& entry : fs::recursive_directory_iterator(directory))
        {
            if (entry.path().extension() != ".rfx")
                continue;

            DYNAMIC_SECTION(entry.path().stem().u8string())
            {
                runTestOnFile(entry, directory);
            }
        }
    }

}