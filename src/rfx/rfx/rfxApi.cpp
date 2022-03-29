#include "include/rfx.hpp"

#include "compiler/DiagnosticSink.hpp"
#include "compiler/Preprocessor.hpp"

#include "core/Blob.hpp"
#include "core/Error.hpp"
#include "core/FileSystem.hpp"
#include "core/IncludeSystem.hpp"
#include "core/SourceLocation.hpp"
#include "core/StringEscapeUtil.hpp"

namespace RR::Rfx
{

    class SourceWriter
    {
    public:
        void Emit(const Token& token)
        {
            U8String escapedToken;
            StringEscapeUtil::AppendEscaped(StringEscapeUtil::Style::JSON, token.stringSlice, escapedToken);

            if (token.type == Token::Type::RBrace)
                dedent();

            if (IsSet(token.flags, Token::Flags::AtStartOfLine))
            {
                if (!output_.empty()) // Skip new line at the wery begining
                    output_ += "\n" + indentString_;
            }
            else if (IsSet(token.flags, Token::Flags::AfterWhitespace))
            {
                output_ += " ";
            }

            output_.append(token.stringSlice.Begin(), token.stringSlice.End());

            if (token.type == Token::Type::LBrace)
                intend();
        }

        U8String GetOutput()
        {
            return output_;
        }

    private:
        inline void intend()
        {
            indentLevel_++;
            updateIndentStiring();
        }

        inline void dedent()
        {
            ASSERT(indentLevel_ > 0);

            if (indentLevel_ == 0)
                return;

            indentLevel_--;
            updateIndentStiring();
        }

        void updateIndentStiring()
        {
            indentString_ = "";

            for (uint32_t i = 0; i < indentLevel_; i++)
                indentString_ += "    ";
        }

    private:
        uint32_t indentLevel_ = 0;
        U8String indentString_ = "";
        U8String output_;
    };

    std::string writeTokens(const std::shared_ptr<Preprocessor>& preprocessor)
    {
        const auto& tokens = preprocessor->ReadAllTokens();
        SourceWriter writer;

        for (const auto& token : tokens)
            writer.Emit(token);

        return writer.GetOutput();
    }

    class ComplileResult : public ICompileResult, RefObject
    {
    public:
        uint32_t addRef() override { return addReference(); }
        uint32_t release() override { return releaseReference(); }

        void PushOutput(CompileOutputType type, const ComPtr<IBlob>& blob)
        {
            outputs.push_back({ type, blob });
        }

        RfxResult GetOutput(size_t index, CompileOutputType& outputType, IBlob** output) override
        {
            assert(index < GetOutputsCount());

            if (index >= GetOutputsCount())
                return RfxResult::InvalidArgument;

            outputs[index].blob.copyTo(output);
            outputType = outputs[index].type;
            return RfxResult::Ok;
        }

        size_t GetOutputsCount() override { return outputs.size(); }

    private:
        struct Output
        {
            CompileOutputType type;
            ComPtr<IBlob> blob;
        };

    private:
        std::vector<Output> outputs;
    };

    RFX_API RfxResult Compile(const CompilerRequestDescription& compilerRequest, ICompileResult** outCompilerResult)
    {
        RfxResult result = RfxResult::Ok;

        if (!outCompilerResult)
            return RfxResult::InvalidArgument;

        if (!compilerRequest.inputFile)
            return RfxResult::InvalidArgument;

        ComPtr<ComplileResult> compilerResult(new ComplileResult());
        PathInfo pathInfo;

        const auto& fileSystem = std::make_shared<OSFileSystem>();
        const auto& includeSystem = std::make_shared<IncludeSystem>(fileSystem);
        if (RFX_FAILED(result = includeSystem->FindFile(compilerRequest.inputFile, "", pathInfo)))
            return result;

        std::shared_ptr<RR::Rfx::SourceFile> sourceFile;
        if (RFX_FAILED(result = includeSystem->LoadFile(pathInfo, sourceFile)))
            return result;

        auto diagnosticSink = std::make_shared<DiagnosticSink>();

        auto bufferWriter = std::make_shared<BufferWriter>();
        diagnosticSink->AddWriter(bufferWriter);

        const auto& preprocessor = std::make_shared<Preprocessor>(includeSystem, diagnosticSink);
        preprocessor->PushInputFile(sourceFile);

        if (compilerRequest.outputPreprocessorResult)
        {
            const auto& blob = ComPtr<IBlob>(new Blob(writeTokens(preprocessor)));
            compilerResult->PushOutput(CompileOutputType::Preprocesed, blob);
        }

        *outCompilerResult = compilerResult.detach();
        return result;
    }

    RFX_API RfxResult GetErrorMessage(RfxResult result, IBlob** message)
    {
        if (!message)
            return RfxResult::InvalidArgument;

        auto blob = ComPtr<IBlob>(new Blob(GetErrorMessage(result)));
        *message = blob.detach();
        return RfxResult::Ok;
    }
}