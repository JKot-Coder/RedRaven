#pragma once

#include <cassert>
#include <stdint.h>
#include <type_traits>
#include <utility>

#ifdef _MSC_VER
#define RFX_STDCALL __stdcall
#ifdef RFX_DYNAMIC_EXPORT
#define RFX_DLL_EXPORT __declspec(dllexport)
#else
#define RFX_DLL_EXPORT __declspec(dllimport)
#endif
#else
#define RFX_STDCALL
#define RFX_DLL_EXPORT __attribute__((visibility("default")))
#endif

#define RFX_API RFX_DLL_EXPORT

#ifdef __cplusplus
#define RFX_EXTERN_C extern "C"
#else
#define RFX_EXTERN_C
#endif

namespace RR
{
    namespace Common
    {
        enum class RResult : int32_t;
    }

    namespace Rfx
    {
        using RfxResult = Common::RResult;

        enum class CompileTarget : uint32_t
        {
            Dxil,
            Dxil_asm,

            Count
        };

        struct CompileRequestDescription
        {
            enum class OutputStage : uint32_t
            {
                Lexer,
                Preprocessor,
                Compiler,
            };

            OutputStage outputStage;

            const char* inputFile;
            const char** defines;
            size_t defineCount;

            struct CompilerOptions
            {
                bool assemblyOutput;
                bool objectOutput;
            } compilerOptions;
        };

        class IRfxUnknown
        {
        public:
            virtual uint32_t AddRef() = 0;
            virtual uint32_t Release() = 0;
        };

        class IBlob : public IRfxUnknown
        {
        public:
            virtual const void* GetBufferPointer() const = 0;
            virtual size_t GetBufferSize() const = 0;
        };

        enum class CompileOutputType
        {
            Diagnostic,
            Assembly,
            Tokens, // The code after lexer stage
            Source, // The code after preprossing stage
            Object
        };

        class ICompileResult : public IRfxUnknown
        {
        public:
            virtual RfxResult GetOutput(size_t index, CompileOutputType& outputType, IBlob** output) = 0;
            virtual size_t GetOutputsCount() = 0;
        };

        class ICompiler : public IRfxUnknown
        {
        public:
            virtual RfxResult Compile(const CompileRequestDescription& compileRequest, ICompileResult** result) = 0;
        };

        RFX_EXTERN_C RFX_API RfxResult RFX_STDCALL GetComplierInstance(ICompiler** compiler);
    }
}