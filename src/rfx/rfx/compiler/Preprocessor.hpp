#pragma once

namespace RR
{
    namespace Rfx
    {
        class SourceFile;
        class DiagnosticSink;
        class IncludeSystem;
        struct Token;
        class PreprocessorImpl;

        class Preprocessor final
        {
        public:
            Preprocessor() = delete;
            Preprocessor(const std::shared_ptr<IncludeSystem>& includeSystem,
                         const std::shared_ptr<DiagnosticSink>& diagnosticSink);

            ~Preprocessor();

            void PushInputFile(const std::shared_ptr<SourceFile>& sourceFile);

            // read the entire input into tokens
            std::vector<Token> ReadAllTokens();

        private:
            // TODO tempoprary shared. Is it possible not to use it?
            std::shared_ptr<PreprocessorImpl> impl_;
        };
    }
}