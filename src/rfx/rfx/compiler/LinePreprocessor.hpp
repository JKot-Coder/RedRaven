#pragma once

namespace RR
{
    namespace Common
    {
        class LinearAllocator;
    }

    namespace Rfx
    {
        class SourceFile;
        class DiagnosticSink;
        struct CompileContext;
        struct Token;
        class LinePreprocessorImpl;

        class LinePreprocessor final
        {
        public:
            LinePreprocessor() = delete;
            LinePreprocessor(const std::shared_ptr<CompileContext>& context);

            ~LinePreprocessor();

            void PushInputFile(const std::shared_ptr<SourceFile>& sourceFile);
            void DefineMacro(const U8String& key, const U8String& value);

            // read the entire input into tokens
            std::vector<Token> ReadAllTokens();

        private:
            // TODO tempoprary shared. Is it possible not to use it?
            std::shared_ptr<LinePreprocessorImpl> impl_;
        };
    }
}