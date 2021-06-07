#include "CompileRequest.hpp"

#include "compiler/Program.hpp"

#include <slang.h>

#include "include/rfx.hpp"

namespace
{
    struct CompileTargetConversion
    {
        Rfx::CompileTarget from;
        SlangCompileTarget to;
    };

    // clang-format off
    static CompileTargetConversion compileTargetConversion[] = {
        { Rfx::CompileTarget::Dxil,             SLANG_DXIL },
        { Rfx::CompileTarget::Dxil_asm,         SLANG_DXIL_ASM },
    }; // clang-format on

    static_assert(std::is_same<std::underlying_type<Rfx::CompileTarget>::type, uint32_t>::value);
    static_assert(std::size(compileTargetConversion) == static_cast<uint32_t>(Rfx::CompileTarget::Count));

    SlangCompileTarget getCompileTargetConversion(Rfx::CompileTarget target)
    {
        ASSERT(compileTargetConversion[static_cast<uint32_t>(target)].from == target);
        return compileTargetConversion[static_cast<uint32_t>(target)].to;
    }
}

namespace Rfx
{
    namespace Compiler
    {
        CompileRequest::CompileRequest(const ::Slang::ComPtr<slang::IGlobalSession>& globalSesion)
        {
            ASSERT(globalSesion);

            slang::SessionDesc desc = {};
            globalSesion->createSession(desc, session_.writeRef());
            ASSERT(session_);
        }

        CompileRequest::~CompileRequest()
        {
            clearModules();
        }

        void CompileRequest::clearModules()
        {
            components_.clear();
        }

        bool CompileRequest::LoadModule(const std::string& name, std::string& log)
        {
            ASSERT(session_);

            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            slang::IModule* module = session_->loadModule(name.c_str(), diagnosticsBlob.writeRef());

            if (diagnosticsBlob)
                log += (const char*)diagnosticsBlob->getBufferPointer();

            if (!module)
                return false;

            components_.push_back(module);

            return true;
        }

        bool CompileRequest::AddEntryPoint()
        {
            ASSERT(session_);

            for (auto component : components_)
            {
                slang::IModule* module;
                if (SLANG_FAILED(component->queryInterface(slang::IModule::getTypeGuid(), (void**)&module)))
                    continue;

                Slang::ComPtr<slang::IEntryPoint> entryPoint;
                if (SLANG_FAILED(module->findEntryPointByName("weq", entryPoint.writeRef())))
                    continue;
            }

            return false;
        }

        Program::SharedPtr CompileRequest::Compile(std::string& log)
        {
            ASSERT(session_);

            Slang::ComPtr<slang::IComponentType> composedProgram;
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;

            const auto result = session_->createCompositeComponentType(
                components_.empty() ? nullptr : &components_.front(),
                components_.size(),
                composedProgram.writeRef(),
                diagnosticsBlob.writeRef());

            if (diagnosticsBlob)
                log += (const char*)diagnosticsBlob->getBufferPointer();

            if (SLANG_FAILED(result))
            {
                clearModules();
                session_.setNull();
                return nullptr;
            }

            return Program::SharedPtr(new Program(composedProgram));
        }
    }
}