#include "DxcPreprocessor.hpp"

#include "core/Blob.hpp"
#include "core/CStringAllocator.hpp"
#include "core/SourceLocation.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

#include "dxcapi.use.h"
//#include <winrt/base.h>

#include "common/Result.hpp"

namespace RR
{
    namespace Rfx
    {
        RfxResult DxcPreprocessor::Preprocess(std::shared_ptr<RR::Rfx::SourceFile>& source, ComPtr<IBlob>& output, ComPtr<IBlob>& diagnostic)
        {
            dxc::DxcDllSupport dxcDll;

            if (FAILED(dxcDll.Initialize()))
                return RfxResult::InternalFail;

            ComPtr<IDxcCompiler3> dxcCompiler;
            if (FAILED(dxcDll.CreateInstance(CLSID_DxcCompiler, dxcCompiler.put())))
                return RfxResult::InternalFail;

            std::vector<LPCWSTR> arguments;
            arguments.push_back(L"-P");
            arguments.push_back(L"preprocessed.hlsl");

            CStringAllocator<wchar_t> cstringAllocator;
            wchar_t* cstring = nullptr;

            for (const auto& define : defines)
            {
                arguments.push_back(L"-D");

                cstring = cstringAllocator.Allocate(StringConversions::UTF8ToWString(define));
                arguments.push_back(cstring);
            }

            cstring = cstringAllocator.Allocate(StringConversions::UTF8ToWString(source->GetPathInfo().getMostUniqueIdentity()));
            arguments.push_back(cstring);

            DxcBuffer dxcSource;
            dxcSource.Ptr = source->GetContent().Begin();
            dxcSource.Size = source->GetContentSize();
            dxcSource.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.

            ComPtr<IDxcResult> dxcResult;
            if (FAILED(dxcCompiler->Compile(&dxcSource, arguments.data(), uint32_t(arguments.size()), nullptr, IID_PPV_ARGS(dxcResult.put()))))
                return RfxResult::InternalFail;

            auto convertBlob = [dxcResult](DXC_OUT_KIND from, ComPtr<IBlob>& output)
            {
                ComPtr<IDxcBlobUtf8> dxcBlobUtf8;

                if (FAILED(dxcResult->GetOutput(from, IID_PPV_ARGS(dxcBlobUtf8.put()), nullptr)))
                    return RfxResult::InternalFail;

                output = ComPtr<IBlob>(new Blob(dxcBlobUtf8->GetStringPointer()));
                return RfxResult::Ok;
            };

            if (FAILED(convertBlob(DXC_OUT_HLSL, output)))
                return RfxResult::InternalFail;

            if (FAILED(convertBlob(DXC_OUT_ERRORS, diagnostic)))
                return RfxResult::InternalFail;

            return RfxResult::Ok;
        };
    }
}