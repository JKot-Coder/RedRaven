#include "TestUtils.hpp"

#include "common/Result.hpp"

#include "rfx/compiler/CompileContext.hpp"
#include "rfx/compiler/DiagnosticCore.hpp"
#include "rfx/compiler/Lexer.hpp"
#include "rfx/core/FileSystem.hpp"
#include "rfx/core/IncludeSystem.hpp"

#include <catch2/catch.hpp>

namespace RR::Rfx
{
    namespace Tests
    {
        class LexerTokenReader
        {
        public:
            LexerTokenReader(const std::shared_ptr<RR::Rfx::SourceFile>& sourceFile, const std::shared_ptr<RR::Rfx::CompileContext>& context)
                : lexer(SourceView::Create(sourceFile), context)
            {
            }

            Token ReadToken() { return lexer.ReadToken(); }

            TokenList ReadAllTokens()
            {
                TokenList tokenList;

                for (;;)
                {
                    const auto& token = ReadToken();

                    switch (token.type)
                    {
                        default: break;
                        case Token::Type::WhiteSpace:
                        case Token::Type::BlockComment:
                        case Token::Type::LineComment:
                        case Token::Type::NewLine:
                            continue;
                    }

                    tokenList.push_back(token);

                    if (token.type == Token::Type::EndOfFile)
                        return tokenList;
                }
            }

        private:
            Lexer lexer;
        };

        TEST_CASE("LexerTests", "[Lexer]")
        {
            const auto fileSystem = std::make_shared<OSFileSystem>();
            const auto includeSystem = std::make_shared<IncludeSystem>(fileSystem);

            auto context = std::make_shared<CompileContext>();

            auto qwe = [&includeSystem, &context](const U8String& filepath, const U8String& testDirectory)
            {
                PathInfo pathInfo;
                RR_RETURN_ON_FAIL(includeSystem->FindFile(filepath, testDirectory, pathInfo));
                std::shared_ptr<RR::Rfx::SourceFile> sourceFile;
                RR_RETURN_ON_FAIL(includeSystem->LoadFile(pathInfo, sourceFile));

                LexerTokenReader reader(sourceFile, context);
                reader.ReadAllTokens();

                return Common::RResult::Ok;
            };

            //runTestOnFile("../src/rfx/tests/lexer/test.rfx", "../src/rfx/tests/lexer", TestType::LexerTest);
            runTestsInDirectory2("../src/rfx/tests/lexer", qwe);
        }
    }
}