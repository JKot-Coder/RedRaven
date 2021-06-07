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
            auto programReflection = composedProgram_->getLayout();
            for (uint32_t i = 0; i < programReflection->getEntryPointCount(); i++)
            {
              //  auto entryPointInfo = programReflection->getEntryPointByIndex(i);
               // auto stage = entryPointInfo->getStage();
                Slang::ComPtr<ISlangBlob> kernelCode;
                Slang::ComPtr<ISlangBlob> diagnostics;
                auto compileResult = composedProgram_->getEntryPointCode(
                    i, 0, kernelCode.writeRef(), diagnostics.writeRef());

                if (diagnostics)
                    log = (char*)diagnostics->getBufferPointer();

                if (!compileResult)
                    return false;
            }

            return true;
        }
    }
}
