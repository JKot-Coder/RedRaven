#include "include/rfx.hpp"

#include "compiler/DiagnosticSink.hpp"
#include "compiler/Lexer.hpp"
#include "compiler/Preprocessor.hpp"

#include "core/Blob.hpp"
#include "core/Error.hpp"
#include "core/FileSystem.hpp"
#include "core/IncludeSystem.hpp"
#include "core/SourceLocation.hpp"
#include "core/StringEscapeUtil.hpp"

#include "common/LinearAllocator.hpp"

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

    U8String writeTokens(const std::shared_ptr<Preprocessor>& preprocessor)
    {
        const auto& tokens = preprocessor->ReadAllTokens();
        SourceWriter writer;

        for (const auto& token : tokens)
            writer.Emit(token);

        return writer.GetOutput();
    }

    U8String writeTokens(const std::shared_ptr<Lexer>& lexer)
    {
        U8String result;

        while (true)
        {
            const auto& token = lexer->ReadToken();

            U8String escapedToken;
            StringEscapeUtil::AppendEscaped(StringEscapeUtil::Style::JSON, token.stringSlice, escapedToken);

            result += fmt::format("{{\"Type\":\"{0}\", \"Content\":\"{1}\", \"Line\":{2}, \"Column\":{3}}},\n",
                                  RR::Rfx::TokenTypeToString(token.type),
                                  escapedToken,
                                  token.humaneSourceLocation.line,
                                  token.humaneSourceLocation.column);

            if (token.type == Token::Type::EndOfFile)
                break;
        }

        return result;
    }

    class ComplileResult : public ICompileResult, RefObject
    {
    public:
        uint32_t addRef() override { return addReference(); }
        uint32_t release() override { return releaseReference(); }

        void SetDiagnosticOutput(const ComPtr<IBlob>& diagnosticBlob)
        {
            diagnosticBlob_ = diagnosticBlob;
        }

        void PushOutput(CompileOutputType type, const ComPtr<IBlob>& blob)
        {
            outputs_.push_back({ type, blob });
        }

        RfxResult GetDiagnosticOutput(IBlob** output) override
        {
            if (!output)
                return RfxResult::InvalidArgument;

            if (!diagnosticBlob_)
                return RfxResult::Fail;

            diagnosticBlob_.copyTo(output);
            return RfxResult::Ok;
        }

        RfxResult GetOutput(size_t index, CompileOutputType& outputType, IBlob** output) override
        {
            assert(index < GetOutputsCount());

            if (!output)
                return RfxResult::InvalidArgument;

            if (index >= GetOutputsCount())
                return RfxResult::InvalidArgument;

            outputs_[index].blob.copyTo(output);
            outputType = outputs_[index].type;
            return RfxResult::Ok;
        }

        size_t GetOutputsCount() override { return outputs_.size(); }

    private:
        struct Output
        {
            CompileOutputType type;
            ComPtr<IBlob> blob;
        };

    private:
        std::vector<Output> outputs_;
        ComPtr<IBlob> diagnosticBlob_;
    };

    RFX_API RfxResult Compile(const CompilerRequestDescription& compilerRequest, ICompileResult** outCompilerResult)
    {
        RfxResult result = RfxResult::Ok;

        if (!outCompilerResult)
            return RfxResult::InvalidArgument;

        if (!compilerRequest.inputFile)
            return RfxResult::InvalidArgument;

        try
        {
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

            if (compilerRequest.lexerOutput)
            {
                // Re-running lexing here leads to duplicate diagnostic messages in the sink
                // when lexerOutput is requested along with other compilation steps.
                // Fortunately, lexerOutput is needed only for testing purposes
                // and this problem is not critical.

                const auto linearAllocator = std::make_shared<LinearAllocator>(1024);
                const auto sourceView = RR::Rfx::SourceView::Create(sourceFile);
                const auto& lexer = std::make_shared<Lexer>(sourceView, linearAllocator, diagnosticSink);

                const auto& blob = ComPtr<IBlob>(new Blob(writeTokens(lexer)));
                compilerResult->PushOutput(CompileOutputType::Lexer, blob);
            }

            if (compilerRequest.preprocessorOutput)
            {
                const auto& preprocessor = std::make_shared<Preprocessor>(includeSystem, diagnosticSink);
                preprocessor->PushInputFile(sourceFile);

                const auto& blob = ComPtr<IBlob>(new Blob(writeTokens(preprocessor)));
                compilerResult->PushOutput(CompileOutputType::Preprocessor, blob);
            }

            // Todo optimize reduce copy;
            const auto& diagnosticBlob = ComPtr<IBlob>(new Blob(bufferWriter->GetBuffer()));
            compilerResult->SetDiagnosticOutput(diagnosticBlob);

            *outCompilerResult = compilerResult.detach();
        }
        catch (const utf8::exception& e)
        {
            Log::Format::Error("UTF8 exception: {}\n", e.what());
            return RfxResult::CannotOpen;
        }

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