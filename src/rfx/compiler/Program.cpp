#include "Program.hpp"

namespace Rfx
{
    namespace Compiler
    {
        Program::Program(const Slang::ComPtr<slang::IComponentType>& composedProgram)
            : composedProgram_(composedProgram)
        {
            ASSERT(composedProgram_);
        }

        bool Program::GetShaderProgram(std::string& log)
        {
            Slang::ComPtr<ISlangBlob> diagnostics;
            auto programReflection = composedProgram_->getLayout(0, diagnostics.writeRef());

            if (diagnostics)
            {
                log += (char*)diagnostics->getBufferPointer();
                return false;
            }

            for (uint32_t i = 0; i < programReflection->getEntryPointCount(); i++)
            {
                //  auto entryPointInfo = programReflection->getEntryPointByIndex(i);
                // auto stage = entryPointInfo->getStage();
                Slang::ComPtr<ISlangBlob> kernelCode;

                auto compileResult = composedProgram_->getEntryPointCode(
                    i, 0, kernelCode.writeRef(), diagnostics.writeRef());

                const auto code = kernelCode->getBufferPointer();
                const auto size = kernelCode->getBufferSize();

                std::ignore = code;
                std::ignore = size;

                if (diagnostics)
                    log += (char*)diagnostics->getBufferPointer(); 

                if (!compileResult)
                    return false;
            }

            return true;
        }
    }
}
