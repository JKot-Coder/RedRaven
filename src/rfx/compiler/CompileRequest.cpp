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
/*
    SlangCompileTarget getCompileTargetConversion(Rfx::CompileTarget target)
    {
        ASSERT(compileTargetConversion[static_cast<uint32_t>(target)].from == target);
        return compileTargetConversion[static_cast<uint32_t>(target)].to;
    }
    */
}

namespace Rfx
{
    namespace Compiler
    {
        CompileRequest::CompileRequest(const ::Slang::ComPtr<slang::IGlobalSession>& globalSesion)
        {
            ASSERT(globalSesion);

            slang::TargetDesc targetDesc = {};
            targetDesc.format = SLANG_DXBC_ASM;
            targetDesc.profile = globalSesion->findProfile("sm_5_0");
            targetDesc.optimizationLevel = SLANG_OPTIMIZATION_LEVEL_MAXIMAL;
            targetDesc.floatingPointMode = SLANG_FLOATING_POINT_MODE_DEFAULT;
            targetDesc.lineDirectiveMode = SLANG_LINE_DIRECTIVE_MODE_DEFAULT;
            targetDesc.flags = 0;

            slang::SessionDesc desc = {};
            desc.targets = &targetDesc;
            desc.targetCount = 1;
            globalSesion->createSession(desc, session_.writeRef());
            ASSERT(session_);

            session_->createCompileRequest(request_.writeRef());
            ASSERT(request_);
        }

        CompileRequest::~CompileRequest()
        {
            clearModules();
        }

        void CompileRequest::clearModules()
        {
            // components_.clear();
        }

        bool CompileRequest::LoadModule(const std::string& name, std::string& log)
        {
            ASSERT(session_);

            std::ignore = log;

            const auto index = request_->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
            request_->addTranslationUnitSourceFile(index, name.c_str());

            return true;
        }

        bool CompileRequest::AddEntryPoint()
        {
            ASSERT(session_);

            return true;
        }

        Program::SharedPtr CompileRequest::Compile(std::string& log)
        {
            ASSERT(session_);

            auto entryPointIndex = request_->addEntryPoint(0, "mainCS", SLANG_STAGE_COMPUTE);
            ::Slang::ComPtr<slang::IComponentType> entryPoint;

            if (SLANG_FAILED(request_->compile()))
            {
                log = request_->getDiagnosticOutput();
                clearModules();
                request_.setNull();
                session_.setNull();
                return nullptr;
            }

            // Slang::ComPtr<slang::IComponentType> composedProgram;
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;

            if (diagnosticsBlob)
                log = (const char*)diagnosticsBlob->getBufferPointer();

            //         if (SLANG_FAILED(result))

            Slang::ComPtr<slang::IComponentType> composedProgram;
            if (SLANG_FAILED(request_->getProgram(composedProgram.writeRef())))
            {
                log = "";
                return nullptr;
            }

            std::ignore = entryPointIndex;
           
            const auto result = request_->getEntryPoint(entryPointIndex, entryPoint.writeRef());
            (void) result;
            ASSERT(SLANG_SUCCEEDED(request_->getEntryPoint(entryPointIndex, entryPoint.writeRef())));

            std::vector<slang::IComponentType*> components;
            components.push_back(composedProgram);
            components.push_back(entryPoint);
            /*
            if (SLANG_FAILED(entryPoint->link(composedProgram.writeRef(), diagnosticsBlob.writeRef())))
            {
                log = "";
                return nullptr;
            }
         */
            if (SLANG_FAILED(session_->createCompositeComponentType(
                    components.data(),
                    components.size(),
                    composedProgram.writeRef(),
                    diagnosticsBlob.writeRef())))
            {
                log = "";
                return nullptr;
            }

            //   entryPoint->link(linkedProgram.writeRef());

            return Program::SharedPtr(new Program(composedProgram));
        }
    }
}