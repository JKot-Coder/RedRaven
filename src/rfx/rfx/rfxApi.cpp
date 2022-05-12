#include "include/rfx.hpp"

#include "compiler/DiagnosticSink.hpp"
#include "compiler/Lexer.hpp"

#include "core/Blob.hpp"
#include "core/Error.hpp"
#include "core/FileSystem.hpp"
#include "core/IncludeSystem.hpp"
#include "core/SourceLocation.hpp"
#include "core/StringEscapeUtil.hpp"

#include "common/LinearAllocator.hpp"

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
#include <winrt/base.h>

namespace RR::Rfx
{
    namespace
    {

        template <typename T>
        class CStringAllocator final
        {
        public:
            ~CStringAllocator()
            {
                for (const auto cstring : cstring_)
                    delete[] cstring;
            }

            void Allocate(const std::vector<std::basic_string<T>>& strings)
            {
                for (const auto& string : strings)
                    Allocate(string);
            }

            T* Allocate(const std::basic_string<T>& string)
            {
                const auto stringLength = string.length();

                auto cString = new T[stringLength + 1];
                string.copy(cString, stringLength);
                cString[stringLength] = '\0';

                cstring_.push_back(cString);
                return cString;
            }

            const std::vector<T*>& GetCStrings() const { return cstring_; }

        private:
            std::vector<T*> cstring_;
        };

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

            if (IsSet(token.flags, Token::Flags::AtStartOfLine))
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
        uint32_t line_ = 1;
        uint32_t indentLevel_ = 0;
        U8String indentString_ = "";
        U8String output_;
        U8String uniqueP;
        std::shared_ptr<SourceView> currentSourceView_;
        std::shared_ptr<SourceFile> currentSourceFile_;
    };

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
        Compiler()
        {
            dxcDll.Initialize();
        }

    public:
        ~Compiler() { instance_ = nullptr; }

        uint32_t addRef() override { return addReference(); }
        uint32_t release() override { return releaseReference(); }

        RfxResult Compile(const CompileRequestDescription& compilerRequest, ICompileResult** result) override;

    public:
        static ICompiler* GetInstance()
        {
            if (!instance_)
                instance_ = new Compiler();

            return instance_;
        };

    private:
        static ICompiler* instance_;

    private:
        dxc::DxcDllSupport dxcDll;
    };

    ICompiler* Compiler::instance_ = nullptr;

    RfxResult Compiler::Compile(const CompileRequestDescription& compilerRequest, ICompileResult** outCompilerResult)
    {
        RfxResult result = RfxResult::Ok;

        if (!outCompilerResult)
            return RfxResult::InvalidArgument;

        if (!compilerRequest.inputFile)
            return RfxResult::InvalidArgument;

        if (compilerRequest.defineCount > 0 && !compilerRequest.defines)
            return RfxResult::InvalidArgument;

        winrt::com_ptr<IDxcCompiler3> dxcCompiler;
        if (FAILED(dxcDll.CreateInstance(CLSID_DxcCompiler, dxcCompiler.put())))
            return RfxResult::InternalFail;

        winrt::com_ptr<IDxcUtils> dxcUtils;
        if (FAILED(dxcDll.CreateInstance(CLSID_DxcUtils, dxcUtils.put())))
            return RfxResult::InternalFail;

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

            // auto bufferWriter = std::make_shared<BufferWriter>();
            //  diagnosticSink->AddWriter(bufferWriter);
            winrt::com_ptr<IDxcResult> dxcResult;

            switch (compilerRequest.outputStage)
            {
                case CompileRequestDescription::OutputStage::Lexer:

                    break;

                case CompileRequestDescription::OutputStage::Compiler:
                case CompileRequestDescription::OutputStage::Preprocessor:
                {
                    DxcBuffer source;
                    source.Ptr = sourceFile->GetContent().Begin();
                    source.Size = sourceFile->GetContentSize();
                    source.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.

                    CStringAllocator<wchar_t> cstringAllocator;
                    std::vector<LPCWSTR> arguments;

                    if (compilerRequest.outputStage == CompileRequestDescription::OutputStage::Preprocessor)
                    {
                        arguments.push_back(L"-P");
                        arguments.push_back(L"preprocessed.hlsl");
                    }

                    bool outputAssembly = false;
                    bool outputObject = false;
                    if (compilerRequest.outputStage == CompileRequestDescription::OutputStage::Compiler)
                    {
                        outputObject = compilerRequest.compilerOptions.objectOutput;
                        outputAssembly = compilerRequest.compilerOptions.assemblyOutput;

                        arguments.push_back(L"-T");
                        arguments.push_back(L"ps_6_0");

                        arguments.push_back(L"-E");
                        arguments.push_back(L"main");
                    }

                    wchar_t* cstring = nullptr;

                    for (size_t index = 0; index < compilerRequest.defineCount; index++)
                    {
                        const auto& define = *(compilerRequest.defines + index);

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

                        winrt::com_ptr<IDxcBlobUtf8> preprocessedBlob;
                        HRESULT hresult = dxcResult->GetOutput(output, IID_PPV_ARGS(preprocessedBlob.put()), nullptr);

                        LPCSTR qwe = nullptr;

                        if (preprocessedBlob)
                            qwe = preprocessedBlob->GetStringPointer();

                        std::ignore = hresult;
                        std::ignore = qwe;
                    }

                    auto convertOutput = [compilerResult](const winrt::com_ptr<IDxcResult>& dxcResult, DXC_OUT_KIND from, CompileOutputType to)
                    {
                        if (from == DXC_OUT_OBJECT)
                        {
                            winrt::com_ptr<IDxcBlob> dxcBlob;

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
                            winrt::com_ptr<IDxcBlobUtf8> dxcBlobUtf8;

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

                    if (compilerRequest.outputStage == CompileRequestDescription::OutputStage::Preprocessor)
                        convertOutput(dxcResult, DXC_OUT_HLSL, CompileOutputType::Source);

                    if (outputObject)
                        convertOutput(dxcResult, DXC_OUT_OBJECT, CompileOutputType::Object);

                    convertOutput(dxcResult, DXC_OUT_ERRORS, CompileOutputType::Diagnostic);

                    if (outputAssembly)
                    {

                        winrt::com_ptr<IDxcBlob> objectBlob;
                        if (SUCCEEDED(dxcResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(objectBlob.put()), nullptr)))
                        {
                            DxcBuffer objectBuffer;
                            objectBuffer.Ptr = objectBlob->GetBufferPointer();
                            objectBuffer.Size = objectBlob->GetBufferSize();
                            objectBuffer.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.

                            winrt::com_ptr<IDxcResult> dissasseblyResult;
                            if (FAILED(dxcCompiler->Disassemble(&objectBuffer, IID_PPV_ARGS(dissasseblyResult.put()))))
                                return RfxResult::InternalFail;

                            convertOutput(dissasseblyResult, DXC_OUT_DISASSEMBLY, CompileOutputType::Assembly);
                        }
                    }
                }
                break;

                default:
                    ASSERT_MSG(false, "Unknown output stage");
                    return RfxResult::InvalidArgument;
            }
            /*
            if (compilerRequest.preprocessorOutput)
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
        *compiler = Compiler::GetInstance();
        (*compiler)->addRef();
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