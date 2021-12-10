#pragma once

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            class SourceFile;
            class DiagnosticSink;
            class Lexer;
            struct Token;

            class Preprocessor final
            {
            public:
                Preprocessor() = delete;
                Preprocessor(const std::shared_ptr<SourceFile>& sourceFile, const std::shared_ptr<DiagnosticSink>& diagnosticSink);
                ~Preprocessor();

                // read the entire input into tokens
                std::shared_ptr<std::vector<Token>> ReadAllTokens();
                Token Preprocessor::ReadToken();

            private:
                struct InputFile;

            private:
                /// Push a new input file onto the input stack of the preprocessor
                void pushInputFile(const std::shared_ptr<InputFile>& inputFile);

                /// Pop the inner-most input file from the stack of input files
                void popInputFile();

            private:
                std::shared_ptr<InputFile> currentInputFile_;
                std::shared_ptr<SourceFile> sourceFile_;
                std::shared_ptr<DiagnosticSink> sink_;
                std::unique_ptr<Lexer> lexer_;
            };
        }
    }
}