#pragma once

namespace RR
{
    namespace ParseTools
    {
        struct CompileContext;
        class SourceFile;
        class SourceManager;
        class DiagnosticSink;
        class IncludeSystem;
        struct Token;
        class PreprocessorImpl;

        class Preprocessor final : Common::NonCopyable
        {
        public:
            Preprocessor() = delete;
            Preprocessor(const std::shared_ptr<IncludeSystem>& includeSystem,
                         const std::shared_ptr<SourceManager>& sourceManager,
                         const std::shared_ptr<CompileContext>& context);

            ~Preprocessor();

            void PushInputFile(const std::shared_ptr<SourceFile>& sourceFile);
            void DefineMacro(const std::string& macro);
            void DefineMacro(const std::string& key, const std::string& value);

            // read the entire input into tokens
            std::vector<Token> ReadAllTokens();

        private:
            std::unique_ptr<PreprocessorImpl> impl_;
        };
    }
}