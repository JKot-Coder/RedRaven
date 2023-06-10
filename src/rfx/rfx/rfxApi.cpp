#include "include/rfx.hpp"

#include "rfx/compiler/CompileContext.hpp"
#include "rfx/compiler/DiagnosticSink.hpp"
#include "rfx/compiler/DxcPreprocessor.hpp"
#include "rfx/compiler/Lexer.hpp"

#include "rfx/core/Blob.hpp"
#include "rfx/core/CStringAllocator.hpp"
#include "rfx/core/FileSystem.hpp"
#include "rfx/core/IncludeSystem.hpp"
#include "rfx/core/SourceLocation.hpp"
#include "rfx/core/StringEscapeUtil.hpp"

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

#include "rfx/compiler/EffectParser.hpp"

#include "dxcapi.use.h"
// #include <winrt/base.h>

namespace RR::Rfx
{
    using RR::Common::ComPtr;

    namespace
    {

        /*
        PathInfo GetIncludePQPEP(const std::shared_ptr<SourceView>& sourceView)
        {
            const auto includeStack = sourceView->GetIncludeStack().GetStack();
            ASSERT(!includeStack.empty())

            for (auto it = includeStack.rbegin(); it != includeStack.rend(); ++it)
            {
                const auto& includeInfo = *it;
                const auto pathInfoType = includeInfo.pathInfo.type;

                if (pathInfoType == PathInfo::Type::Normal ||
                    pathInfoType == PathInfo::Type::FoundPath)
                    return includeInfo.pathInfo;
            }

            return (*std::prev(includeStack.end())).pathInfo;
        }*/

        void qweqwe(const std::shared_ptr<SourceView>& sourceView, std::shared_ptr<SourceView>& outSourceView, HumaneSourceLocation& outHumaneSourceLoc)
        {
            outSourceView = sourceView;

            while (outSourceView->GetInitiatingSourceLocation().GetSourceView() && outSourceView->GetPathInfo().type != PathInfo::Type::Normal)
            {
                outHumaneSourceLoc = outSourceView->GetInitiatingHumaneLocation();
                outSourceView = outSourceView->GetInitiatingSourceLocation().GetSourceView();
            }
        }
    }

    class SourceWriter
    {
    public:
        void Emit(const Token& token)
        {
            U8String escapedToken;
            StringEscapeUtil::AppendEscaped(StringEscapeUtil::Style::JSON, token.stringSlice, escapedToken); //???

            if (token.type == Token::Type::RBrace)
                dedent();

            if (Common::IsSet(token.flags, Token::Flags::AtStartOfLine))
            {
                if (!output_.empty()) // Skip new line at the wery begining
                    output_.push_back('\n');

                if (currentSourceFile_ != token.sourceLocation.GetSourceView()->GetSourceFile())
                {
                    HumaneSourceLocation humaleLoc = token.humaneSourceLocation;
                    std::shared_ptr<SourceView> sourceView;

                    qweqwe(token.sourceLocation.GetSourceView(), sourceView, humaleLoc);
                    line_ = humaleLoc.line;

                    if (!currentSourceFile_ || uniqueP != sourceView->GetSourceFile()->GetPathInfo().getMostUniqueIdentity())
                    {
                        uniqueP = sourceView->GetSourceFile()->GetPathInfo().getMostUniqueIdentity();
                        auto uniquePaa = fs::path(uniqueP);

                        U8String escapedToken2; // TODO  Fix this
                        StringEscapeUtil::AppendEscaped(StringEscapeUtil::Style::Cpp, uniquePaa.make_preferred().u8string(), escapedToken2); //??

                        currentSourceFile_ = token.sourceLocation.GetSourceView()->GetSourceFile();
                        output_ += fmt::format("{}#line {} \"{}\"\n", indentString_, line_, escapedToken2);
                    }
                    else
                    {
                        output_ += fmt::format("{}#line {}\n", indentString_, line_);
                    }
                }
                else
                {
                    HumaneSourceLocation humaleLoc = token.humaneSourceLocation;
                    std::shared_ptr<SourceView> sourceView;

                    qweqwe(token.sourceLocation.GetSourceView(), sourceView, humaleLoc);

                    //  line_ = token.humaneSourceLocation.line;
                    if (humaleLoc.line != line_)
                    {
                        line_ = humaleLoc.line;
                        output_ += fmt::format("{}#line {}\n", indentString_, line_);
                    }
                }

                line_++;
                output_ += indentString_;
            }
            else if (Common::IsSet(token.flags, Token::Flags::AfterWhitespace))
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
        uint32_t line_ = 1;
        uint32_t indentLevel_ = 0;
        U8String indentString_ = "";
        U8String output_;
        U8String uniqueP;
        std::shared_ptr<SourceView> currentSourceView_;
        std::shared_ptr<SourceFile> currentSourceFile_;
    };

    class ComplileResult : public ICompileResult, RefObject
    {
    public:
        uint32_t AddRef() override { return addReference(); }
        uint32_t Release() override { return releaseReference(); }

        void PushOutput(CompileOutputType type, const ComPtr<IBlob>& blob)
        {
            outputs_.push_back({ type, blob });
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
            : lexer_(SourceView::Create(sourceFile), context)
        {
        }

        Token ReadToken() { return lexer_.ReadToken(); }

        ComPtr<IBlob> ReadAllTokens()
        {
            U8String result;

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

                // TODO: really slow implementation
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

            return new Blob(result);
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
            PathInfo pathInfo;

            const auto& fileSystem = std::make_shared<OSFileSystem>();
            const auto& includeSystem = std::make_shared<IncludeSystem>(fileSystem);
            if (RR_FAILED(result = includeSystem->FindFile(compileRequest.inputFile, "", pathInfo)))
                return result;

            std::shared_ptr<RR::Rfx::SourceFile> sourceFile;
            if (RR_FAILED(result = includeSystem->LoadFile(pathInfo, sourceFile)))
                return result;

            ComPtr<IDxcResult> dxcResult;

            switch (compileRequest.outputStage)
            {
                case CompileRequestDescription::OutputStage::Lexer:
                {
                    auto context = std::make_shared<CompileContext>();

                    auto bufferWriter = std::make_shared<BufferWriter>();
                    context->sink.AddWriter(bufferWriter);

                    LexerTokenReader reader(sourceFile, context);
                    const auto sourceBlob = reader.ReadAllTokens();

                    // Todo optimize reduce copy
                    const auto diagnosticBlob = ComPtr<IBlob>(new Blob(bufferWriter->GetBuffer()));
                    compilerResult->PushOutput(CompileOutputType::Diagnostic, diagnosticBlob);
                    compilerResult->PushOutput(CompileOutputType::Source, sourceBlob);
                }
                break;
                case CompileRequestDescription::OutputStage::Compiler:
                case CompileRequestDescription::OutputStage::Preprocessor:
                {
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
                    }

                    EffectParser parser;

                    if (RR_FAILED(rfxResult = parser.Parse(sourceFile, output, diagnostic)))
                        return rfxResult;

                    /*
                        DxcBuffer source;
                    source.Ptr = sourceFile->GetContent().Begin();
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

                        cstring = cstringAllocator.Allocate(StringConversions::UTF8ToWString(define));
                        arguments.push_back(cstring);
                    }

                    cstring = cstringAllocator.Allocate(StringConversions::UTF8ToWString(pathInfo.uniqueIdentity));
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
            *outCompilerResult = compilerResult.detach();
        }
        catch (const utf8::exception& e)
        {
            LOG_ERROR("UTF8 exception: {}", e.what());
            return RfxResult::CannotOpen;
        }

        return result;
    }

    RFX_API RfxResult GetComplierInstance(ICompiler** compiler)
    {
        ASSERT(compiler)

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

        auto blob = ComPtr<IBlob>(new Blob(GetErrorMessage(result)));
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