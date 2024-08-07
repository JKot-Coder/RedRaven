#include "include/rfx.hpp"

#include "rfx/compiler/CompileContext.hpp"
#include "rfx/compiler/DiagnosticSink.hpp"
#include "rfx/compiler/DxcPreprocessor.hpp"
#include "rfx/compiler/EffectCompiler.hpp"
#include "rfx/compiler/Lexer.hpp"
#include "rfx/compiler/Parser.hpp"
#include "rfx/compiler/Preprocessor.hpp"
#include "rfx/compiler/RSONBuilder.hpp"

#include "rfx/core/Blob.hpp"
#include "rfx/core/CStringAllocator.hpp"
#include "rfx/core/FileSystem.hpp"
#include "rfx/core/IncludeSystem.hpp"
#include "rfx/core/SourceLocation.hpp"
#include "rfx/core/StringEscapeUtil.hpp"

#include "common/OnScopeExit.hpp"
#include "common/Result.hpp"

#if !defined(NDEBUG) && OS_WINDOWS
#define ENABLE_LEAK_DETECTION true
#else
#define ENABLE_LEAK_DETECTION false
#endif

#if ENABLE_LEAK_DETECTION
#include "common/debug/LeakDetector.hpp"
#include <Windows.h>
#endif

#ifdef _WIN32
#include <Windows.h>
#endif

#include "dxcapi.use.h"
// #include <winrt/base.h>

#include <stack>

namespace RR::Rfx
{
    using RR::Common::ComPtr;

    namespace
    {
        void qweqwe(const std::shared_ptr<const SourceView>& sourceView, std::shared_ptr<const SourceView>& outSourceView, HumaneSourceLocation& outHumaneSourceLoc)
        {
            outSourceView = sourceView;

            while (outSourceView->GetInitiatingSourceLocation().GetSourceView() &&
                   outSourceView->GetPathInfo().type != PathInfo::Type::Normal &&
                   outSourceView->GetPathInfo().type != PathInfo::Type::Split)
            {
                outHumaneSourceLoc = outSourceView->GetInitiatingSourceLocation().humaneSourceLoc;
                outSourceView = outSourceView->GetInitiatingSourceLocation().GetSourceView();
            }
        }
    }

    template <int IndentCount>
    class SourceWriter
    {
    public:
        SourceWriter() : blob_(new StringBlob()) { }
        virtual ~SourceWriter() { }
        ComPtr<IBlob> GetBlob() { return blob_; }

    protected:
        inline void indent() { indentLevel_++; }
        inline void dedent()
        {
            ASSERT(indentLevel_ > 0);

            if (indentLevel_ == 0)
                return;

            indentLevel_--;
        }

        void writeIndent()
        {
            for (uint32_t i = 0; i < indentLevel_ * IndentCount; i++)
                getOutputString().push_back(' ');
        }

        void push_back(char ch) { getOutputString().push_back(ch); }
        void append(const std::string& str) { getOutputString().append(str); }
        void append(const char* str, size_t count) { getOutputString().append(str, count); }
        template <class InputIterator>
        void append(InputIterator first, InputIterator last) { getOutputString().append(first, last); }

        std::string& getOutputString() { return blob_->GetString(); };

    protected:
        uint32_t indentLevel_ = 0;
        ComPtr<StringBlob> blob_;
    };

    class TokenWriter final : public SourceWriter<4>
    {
    public:
        TokenWriter(bool onlyRelativePaths) : onlyRelativePaths_(onlyRelativePaths) { }

        void Emit(const Token& token)
        {
            if (token.type == Token::Type::RBrace)
                dedent();

            if (Common::IsSet(token.flags, Token::Flags::AtStartOfLine))
            {
                if (!getOutputString().empty()) // Skip new line at the wery begining
                    push_back('\n');

                if (currentSourceFile_ != token.sourceLocation.GetSourceView()->GetSourceFile())
                {
                    HumaneSourceLocation humaleLoc = token.sourceLocation.humaneSourceLoc;
                    std::shared_ptr<const SourceView> sourceView;

                    qweqwe(token.sourceLocation.GetSourceView(), sourceView, humaleLoc);
                    ASSERT(sourceView);

                    line_ = humaleLoc.line;

                    const auto sourceViewUniqueIdentity = sourceView->GetSourceFile()->GetPathInfo().getMostUniqueIdentity();
                    if (!currentSourceFile_ || currentUniqueIndentity_ != sourceViewUniqueIdentity)
                    {
                        currentUniqueIndentity_ = sourceViewUniqueIdentity;
                        auto const path = onlyRelativePaths_ ? sourceView->GetSourceFile()->GetPathInfo().foundPath : sourceViewUniqueIdentity;

                        std::string escapedPath;
                        StringEscapeUtil::AppendEscaped(StringEscapeUtil::Style::Cpp, path, escapedPath);

                        currentSourceFile_ = token.sourceLocation.GetSourceView()->GetSourceFile();
                        writeIndent();
                        append(fmt::format("#line {} \"{}\"\n", line_, escapedPath));
                    }
                    else
                    {
                        writeIndent();
                        append(fmt::format("#line {}\n", line_));
                    }
                }
                else
                {
                    HumaneSourceLocation humaleLoc = token.sourceLocation.humaneSourceLoc;
                    std::shared_ptr<const SourceView> sourceView;

                    qweqwe(token.sourceLocation.GetSourceView(), sourceView, humaleLoc);
                    ASSERT(sourceView);

                    //  line_ = token.humaneSourceLocation.line;

                    constexpr int maxNewLinesInARow = 3;
                    if (humaleLoc.line - line_ <= maxNewLinesInARow)
                    {
                        while (line_ < humaleLoc.line)
                        {
                            line_++;
                            push_back('\n');
                        }
                    }

                    if (humaleLoc.line != line_)
                    {
                        line_ = humaleLoc.line;
                        writeIndent();
                        append(fmt::format("#line {}\n", line_));
                    }
                }

                line_++;
                writeIndent();
            }
            else if (Common::IsSet(token.flags, Token::Flags::AfterWhitespace))
            {
                push_back(' ');
            }

            switch (token.type)
            {
                case Token::Type::StringLiteral:
                case Token::Type::CharLiteral:
                {
                    std::string escapedToken;
                    StringEscapeUtil::AppendEscaped(StringEscapeUtil::Style::Cpp, token.stringSlice, escapedToken);

                    auto appendQuoted = [](char quotingChar, const std::string& token) { return quotingChar + token + quotingChar; };

                    char quotingChar = token.type == Token::Type::StringLiteral ? '\"' : '\'';
                    append(appendQuoted(quotingChar, escapedToken));
                    break;
                }
                default:
                    append(token.stringSlice.begin(), token.stringSlice.end());
                    break;
            }

            if (token.type == Token::Type::LBrace)
                indent();
        }

    private:
        uint32_t line_ = 1;
        std::string currentUniqueIndentity_;
        std::shared_ptr<SourceView> currentSourceView_;
        std::shared_ptr<SourceFile> currentSourceFile_;
        bool onlyRelativePaths_;
    };

    ComPtr<IBlob> writeTokens(const TokenSpan& tokens, const std::shared_ptr<CompileContext>& context)
    {
        TokenWriter writer(context->onlyRelativePaths);

        for (const auto& token : tokens)
            writer.Emit(token);

        return writer.GetBlob();
    }

    class JSONWriter final : public SourceWriter<2>
    {
    public:
        void Write(const RSONValue& value)
        {
            switch (value.type)
            {
                case RSONValue::Type::Bool:
                case RSONValue::Type::Float:
                case RSONValue::Type::Integer:
                case RSONValue::Type::Null:
                case RSONValue::Type::String:
                case RSONValue::Type::Reference:
                    formatPlain(value);
                    break;
                case RSONValue::Type::Array:
                case RSONValue::Type::Object:
                {
                    const bool isArray = value.isArray();
                    const char openBracket = isArray ? '[' : '{';
                    const char closeBracket = isArray ? ']' : '}';

                    push_back(openBracket);
                    push_back('\n');
                    indent();

                    const auto lastElemIter = std::prev(value.end());
                    for (auto iter = value.begin(), end = value.end(); iter != end; ++iter)
                    {
                        const auto& elem = *iter;
                        writeIndent();
                        if (!isArray)
                        {
                            StringEscapeUtil::AppendQuoted(StringEscapeUtil::Style::JSON, elem.first, getOutputString());
                            append(": ", 2);
                        }
                        Write(elem.second);
                        if (lastElemIter != iter)
                            push_back(',');
                        push_back('\n');
                    }
                    dedent();
                    writeIndent();
                    push_back(closeBracket);
                }
                break;
                default:
                    ASSERT_MSG(false, "Unknown type");
            }
        }

    private:
        void formatPlain(const RSONValue& value)
        {
            switch (value.type)
            {
                case RSONValue::Type::Bool: append(value.boolValue ? "true" : "false"); break;
                case RSONValue::Type::Float: append(fmt::format("{}", value.floatValue)); break;
                case RSONValue::Type::Integer: append(fmt::format("{}", value.intValue)); break;
                case RSONValue::Type::Null: append("null", 4); break;
                case RSONValue::Type::String:
                case RSONValue::Type::Reference:
                {
                    StringEscapeUtil::AppendQuoted(StringEscapeUtil::Style::JSON, (value.type == RSONValue::Type::String) ? value.stringValue : value.referenceValue.path, getOutputString());
                    break;
                }
                default: ASSERT_MSG(false, "Unsupported type");
            }
        }
    };

    ComPtr<IBlob> writeJSON(const RSONValue& root)
    {
        JSONWriter writer;
        writer.Write(root);
        return writer.GetBlob();
    }

    class ComplileResult : public ICompileResult, RefObject
    {
    public:
        uint32_t AddRef() override { return addReference(); }
        uint32_t Release() override { return releaseReference(); }

        void PushOutput(CompileOutputType type, const ComPtr<IBlob>& blob)
        {
            outputs_.push_back({type, blob});
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

    class Compiler : public ICompiler, RefObject
    {
    private:
        Compiler() = default;

    public:
        ~Compiler()
        {
            instance_ = nullptr;
            dxcDll.Cleanup();
        }

        uint32_t AddRef() override { return addReference(); }
        uint32_t Release() override { return releaseReference(); }

        RfxResult Compile(const CompileRequestDescription& compileRequest, ICompileResult** result) override;

    public:
        static ICompiler* GetInstance()
        {
            if (!instance_)
            {
                const auto result = dxcDll.Initialize();
                if (FAILED(result))
                {
                    LOG_ERROR("Failed to initialize dxc with error: {0}", GetErrorMessage((Common::RResult)result));
                    return nullptr;
                }

                instance_ = new Compiler();
            }

            return instance_;
        };

    private:
        static ICompiler* instance_;
        static dxc::DxcDllSupport dxcDll;
    };

    dxc::DxcDllSupport Compiler::dxcDll;
    ICompiler* Compiler::instance_ = nullptr;

    class LexerTokenReader
    {
    public:
        LexerTokenReader(const std::shared_ptr<RR::Rfx::SourceFile>& sourceFile, const std::shared_ptr<RR::Rfx::CompileContext>& context)
            : lexer_(SourceView::CreateFromSourceFile(sourceFile), context)
        {
        }

        Token ReadToken() { return lexer_.ReadToken(); }

        ComPtr<IBlob> ReadAllTokens()
        {
            std::string result;

            for (;;)
            {
                const auto& token = ReadToken();

                switch (token.type)
                {
                    default: break;
                    case Token::Type::WhiteSpace:
                    case Token::Type::NewLine:
                        continue;
                }

                std::string tokenString;
                UnownedStringSlice tokenStringSlice = token.stringSlice;

                if (token.type == Token::Type::BlockComment)
                {
                    tokenString = token.stringSlice.asString();

                    std::string::size_type pos = 0;
                    // Make consisten line breaks for multiline comments to pass tests
                    while ((pos = tokenString.find("\r\n", pos)) != std::string::npos)
                        tokenString.replace(pos, 2, "\n");

                    tokenStringSlice = tokenString;
                }

                // TODO: really slow implementation
                std::string escapedToken;
                StringEscapeUtil::AppendEscaped(StringEscapeUtil::Style::JSON, tokenStringSlice, escapedToken);

                result += fmt::format("{{\"Type\":\"{0}\", \"Content\":\"{1}\", \"Line\":{2}, \"Column\":{3}}},\n",
                                      RR::Rfx::TokenTypeToString(token.type),
                                      escapedToken,
                                      token.sourceLocation.humaneSourceLoc.line,
                                      token.sourceLocation.humaneSourceLoc.column);

                if (token.type == Token::Type::EndOfFile)
                    break;
            }

            return new BinaryBlob(result);
        }

    private:
        ComPtr<IBlob> output_;
        Lexer lexer_;
    };

    RfxResult Compiler::Compile(const CompileRequestDescription& compileRequest, ICompileResult** outCompilerResult)
    {
        RfxResult result = RfxResult::Ok;

        if (!outCompilerResult)
            return RfxResult::InvalidArgument;

        if (!compileRequest.inputFile)
            return RfxResult::InvalidArgument;

        if (compileRequest.defineCount > 0 && !compileRequest.defines)
            return RfxResult::InvalidArgument;

        ComPtr<IDxcCompiler3> dxcCompiler;
        if (FAILED(dxcDll.CreateInstance(CLSID_DxcCompiler, dxcCompiler.put())))
            return RfxResult::InternalFail;

        ComPtr<IDxcUtils> dxcUtils;
        if (FAILED(dxcDll.CreateInstance(CLSID_DxcUtils, dxcUtils.put())))
            return RfxResult::InternalFail;

        try
        {
            ComPtr<ComplileResult> compilerResult(new ComplileResult());
            // Always put resutls
            ON_SCOPE_EXIT({ *outCompilerResult = compilerResult.detach(); });
            PathInfo pathInfo;

            const auto& fileSystem = std::make_shared<OSFileSystem>();
            const auto& includeSystem = std::make_shared<IncludeSystem>(fileSystem);
            if (RR_FAILED(result = includeSystem->FindFile(compileRequest.inputFile, "", pathInfo)))
                return result;

            auto context = std::make_shared<CompileContext>(compileRequest.compilerOptions.onlyRelativePaths);
            auto sourceManager = std::make_shared<SourceManager>(context);

            auto bufferWriter = std::make_shared<BufferWriter>();
            context->sink.AddWriter(bufferWriter);

            std::shared_ptr<RR::Rfx::SourceFile> sourceFile;
            if (RR_FAILED(result = sourceManager->LoadFile(pathInfo, sourceFile)))
            {
                const auto diagnosticBlob = ComPtr<IBlob>(new BinaryBlob(bufferWriter->GetBuffer()));
                compilerResult->PushOutput(CompileOutputType::Diagnostic, diagnosticBlob);
                return result;
            }

            ComPtr<IDxcResult> dxcResult;

            switch (compileRequest.outputStage)
            {
                case CompileRequestDescription::OutputStage::Lexer:
                {
                    LexerTokenReader reader(sourceFile, context);
                    const auto sourceBlob = reader.ReadAllTokens();

                    // Todo optimize reduce copy
                    const auto diagnosticBlob = ComPtr<IBlob>(new BinaryBlob(bufferWriter->GetBuffer()));
                    compilerResult->PushOutput(CompileOutputType::Diagnostic, diagnosticBlob);
                    compilerResult->PushOutput(CompileOutputType::Source, sourceBlob);
                }
                break;
                case CompileRequestDescription::OutputStage::Compiler:
                case CompileRequestDescription::OutputStage::Preprocessor:
                case CompileRequestDescription::OutputStage::Parser:
                {
                    Preprocessor preprocessor(includeSystem, sourceManager, context);

                    for (size_t index = 0; index < compileRequest.defineCount; index++)
                    {
                        ASSERT(compileRequest.defines + index);
                        const auto& define = *(compileRequest.defines + index);
                        preprocessor.DefineMacro(define);
                    }

                    // Write diagnostic no matter what
                    ON_SCOPE_EXIT({
                        const auto diagnosticBlob = ComPtr<IBlob>(new BinaryBlob(bufferWriter->GetBuffer()));
                        compilerResult->PushOutput(CompileOutputType::Diagnostic, diagnosticBlob);
                    });

                    preprocessor.PushInputFile(sourceFile);
                    const auto& tokens = preprocessor.ReadAllTokens();

                    if (compileRequest.outputStage == CompileRequestDescription::OutputStage::Preprocessor)
                    {
                        compilerResult->PushOutput(CompileOutputType::Source, writeTokens(tokens, context));
                        break;
                    }
                    else if (compileRequest.outputStage == CompileRequestDescription::OutputStage::Parser ||
                             compileRequest.outputStage == CompileRequestDescription::OutputStage::Compiler)
                    {
                        RSONValue root;

                        {
                            Parser parser(tokens, context);

                            RR_RETURN_ON_FAIL(parser.Parse(root));
                        }

                        if (compileRequest.outputStage == CompileRequestDescription::OutputStage::Parser)
                        {
                            compilerResult->PushOutput(CompileOutputType::Source, writeJSON(root));
                            break;
                        }

                        if (compileRequest.outputStage == CompileRequestDescription::OutputStage::Compiler)
                        {
                            EffectCompiler compiler(context);
                            compiler.Compile(root);
                        }

                        break;
                    }

                    /*
                    DxcPreprocessor preprocessor;

                    for (size_t index = 0; index < compileRequest.defineCount; index++)
                    {
                        ASSERT(compileRequest.defines + index);
                        const auto& define = *(compileRequest.defines + index);
                        preprocessor.DefineMacro(define);
                    }

                    ComPtr<IBlob> output;
                    ComPtr<IBlob> diagnostic;

                    RfxResult rfxResult;
                    if (RR_FAILED(rfxResult = preprocessor.Preprocess(sourceFile, output, diagnostic)))
                        return rfxResult;

                    if (compileRequest.outputStage == CompileRequestDescription::OutputStage::Preprocessor)
                    {
                        compilerResult->PushOutput(CompileOutputType::Source, output);
                        compilerResult->PushOutput(CompileOutputType::Diagnostic, diagnostic);
                    }
                    else
                    {
                        preprocessor.DefineMacro("RFX");

                        if (RR_FAILED(rfxResult = preprocessor.Preprocess(sourceFile, output, diagnostic)))
                            return rfxResult;

                        compilerResult->PushOutput(CompileOutputType::Assembly, output);
                        compilerResult->PushOutput(CompileOutputType::Diagnostic, diagnostic);
                    } */

                    /*
                    EffectParser parser;

                    if (RR_FAILED(rfxResult = parser.Parse(sourceFile, output, diagnostic)))
                        return rfxResult;


                        DxcBuffer source;
                    source.Ptr = sourceFile->GetContent().begin();
                    source.Size = sourceFile->GetContentSize();
                    source.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.

                    CStringAllocator<wchar_t> cstringAllocator;
                    std::vector<LPCWSTR> arguments;

                    if (compileRequest.outputStage == CompileRequestDescription::OutputStage::Preprocessor)
                    {
                        arguments.push_back(L"-P");
                        arguments.push_back(L"preprocessed.hlsl");
                    }

                    bool outputAssembly = false;
                    bool outputObject = false;
                    if (compileRequest.outputStage == CompileRequestDescription::OutputStage::Compiler)
                    {
                        outputObject = compileRequest.compilerOptions.objectOutput;
                        outputAssembly = compileRequest.compilerOptions.assemblyOutput;

                        arguments.push_back(L"-T");
                        arguments.push_back(L"ps_6_0");

                        arguments.push_back(L"-E");
                        arguments.push_back(L"main");
                    }

                    wchar_t* cstring = nullptr;

                    for (size_t index = 0; index < compileRequest.defineCount; index++)
                    {
                        const auto& define = *(compileRequest.defines + index);

                        arguments.push_back(L"-D");

                        cstring = cstringAllocator.Allocate(Common::StringEncoding::UTF8ToWide(define));
                        arguments.push_back(cstring);
                    }

                    cstring = cstringAllocator.Allocate(Common::StringEncoding::UTF8ToWide(pathInfo.uniqueIdentity));
                    arguments.push_back(cstring);

                    if (FAILED(dxcCompiler->Compile(&source, arguments.data(), uint32_t(arguments.size()), nullptr, IID_PPV_ARGS(dxcResult.put()))))
                        return RfxResult::InternalFail;

                    // for (uint32_t i = 0; i < dxcResult->GetNumOutputs(); i++)
                    for (DXC_OUT_KIND output = DXC_OUT_NONE; output <= DXC_OUT_REMARKS; output = (DXC_OUT_KIND)DWORD(output + 1))
                    {
                        //  const auto output = dxcResult->GetOutputByIndex(i);

                        CComPtr<IDxcBlobUtf8> preprocessedBlob;
                        HRESULT hresult = dxcResult->GetOutput(output, IID_PPV_ARGS(preprocessedBlob.put()), nullptr);

                        LPCSTR qwe = nullptr;

                        if (preprocessedBlob)
                            qwe = preprocessedBlob->GetStringPointer();

                        std::ignore = hresult;
                        std::ignore = qwe;
                    }

                    auto convertOutput = [compilerResult](const CComPtr<IDxcResult>& dxcResult, DXC_OUT_KIND from, CompileOutputType to)
                    {
                        if (from == DXC_OUT_OBJECT)
                        {
                            CComPtr<IDxcBlob> dxcBlob;

                            if (SUCCEEDED(dxcResult->GetOutput(from, IID_PPV_ARGS(dxcBlob.put()), nullptr)))
                            {
                                const auto& blob = ComPtr<IBlob>(new Blob(dxcBlob->GetBufferPointer(), dxcBlob->GetBufferSize()));
                                compilerResult->PushOutput(to, blob);
                                return;
                            }
                            else
                                ASSERT_MSG(false, "Empty output");
                        }
                        else
                        {
                            CComPtr<IDxcBlobUtf8> dxcBlobUtf8;

                            if (SUCCEEDED(dxcResult->GetOutput(from, IID_PPV_ARGS(dxcBlobUtf8.put()), nullptr)))
                            {
                                const auto& blob = ComPtr<IBlob>(new Blob(dxcBlobUtf8->GetStringPointer()));
                                compilerResult->PushOutput(to, blob);
                                return;
                            }
                            else
                                ASSERT_MSG(false, "Empty output");
                        }

                        ASSERT_MSG(false, "Empty output");
                    };

                    if (compileRequest.outputStage == CompileRequestDescription::OutputStage::Preprocessor)
                        convertOutput(dxcResult, DXC_OUT_HLSL, CompileOutputType::Source);

                    if (outputObject)
                        convertOutput(dxcResult, DXC_OUT_OBJECT, CompileOutputType::Object);

                    convertOutput(dxcResult, DXC_OUT_ERRORS, CompileOutputType::Diagnostic);

                    if (outputAssembly)
                    {

                        CComPtr<IDxcBlob> objectBlob;
                        if (SUCCEEDED(dxcResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(objectBlob.put()), nullptr)))
                        {
                            DxcBuffer objectBuffer;
                            objectBuffer.Ptr = objectBlob->GetBufferPointer();
                            objectBuffer.Size = objectBlob->GetBufferSize();
                            objectBuffer.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.

                            CComPtr<IDxcResult> dissasseblyResult;
                            if (FAILED(dxcCompiler->Disassemble(&objectBuffer, IID_PPV_ARGS(dissasseblyResult.put()))))
                                return RfxResult::InternalFail;

                            convertOutput(dissasseblyResult, DXC_OUT_DISASSEMBLY, CompileOutputType::Assembly);
                        }
                    }*/
                }
                break;

                default:
                    ASSERT_MSG(false, "Unknown output stage");
                    return RfxResult::InvalidArgument;
            }
            /*
            if (compileRequest.preprocessorOutput)
            {

                //-E for the entry point (eg. PSMain)
                //  arguments.push_back(L"-E");
                // arguments.push_back(entryPoint);

                //-T for the target profile (eg. ps_6_2)
                //arguments.push_back(L"-T");
                //arguments.push_back(target);

                //Strip reflection data and pdbs (see later)
                //  arguments.push_back(L"-Qstrip_debug");
                // arguments.push_back(L"-Qstrip_reflect");

                // arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX
                // arguments.push_back(DXC_ARG_DEBUG); //-Zi
                // arguments.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR); //-Zp
]
            }*/
        }
        catch (const utf8::exception& e)
        {
            LOG_ERROR("UTF8 exception: {}", e.what());
            return RfxResult::Abort;
        }

        return result;
    }

    RFX_API RfxResult GetComplierInstance(ICompiler** compiler)
    {
        ASSERT(compiler);

        *compiler = Compiler::GetInstance();
        if (!*compiler)
            return RfxResult::NoInterface;

        (*compiler)->AddRef();
        return RfxResult::Ok;
    }

    RFX_API RfxResult GetErrorMessage(RfxResult result, IBlob** message)
    {
        if (!message)
            return RfxResult::InvalidArgument;

        auto blob = ComPtr<IBlob>(new BinaryBlob(GetErrorMessage(result)));
        *message = blob.detach();
        return RfxResult::Ok;
    }
}

#if ENABLE_LEAK_DETECTION
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    std::ignore = lpReserved;
    std::ignore = hinstDLL;
    std::ignore = fdwReason;
    std::ignore = RR::Common::Debug::LeakDetector::Instance();
    return TRUE;
}
#endif // OS_WINDOWS