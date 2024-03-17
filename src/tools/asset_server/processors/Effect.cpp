#include "Effect.hpp"

#include "common/Result.hpp"
#include "common/io/File.hpp"

#include "rfx.hpp"

namespace RR::AssetImporter::Processors
{
    REGISTER_PROCESSOR(Effect)

    namespace
    {
        Common::RResult SaveBlob(const std::filesystem::path& path, const Common::ComPtr<Rfx::IBlob>& blob)
        {
            Common::IO::File file;

            const auto result = file.Open(path, Common::IO::FileAccessMode::Create | Common::IO::FileAccessMode::Write);
            if (RR_FAILED(result))
            {
                LOG_WARNING("Failed to save file: \'{0}\' with error: {1}", path.generic_u8string(), Common::GetErrorMessage(result));
                return result;
            }

            if (file.Write(blob->GetBufferPointer(), blob->GetBufferSize()) != blob->GetBufferSize())
                return Common::RResult::Unexpected;

            file.Close();
            return Common::RResult::Ok;
        }
    }

    Effect::Effect()
    {
        if (Rfx::GetComplierInstance(compiler_.put()) != Rfx::RfxResult::Ok)
            LOG_FATAL("Could not get rfx compiler instance");
    }

    std::vector<U8String> Effect::GetListOfExtensions() const
    {
        return { ".rfx" };
    }

    Common::RResult Effect::Process(const Asset& asset, const ProcessorContext& context, std::vector<ProcessorOutput>& outputs) const
    {
        std::ignore = context;

        Rfx::CompileRequestDescription compileRequest {};
        Common::ComPtr<Rfx::ICompileResult> result;

        const auto assetPath = asset.path;
        compileRequest.inputFile = assetPath.u8string().c_str();
        compileRequest.outputStage = Rfx::CompileRequestDescription::OutputStage::Compiler;
        compileRequest.compilerOptions.assemblyOutput = false;
        compileRequest.compilerOptions.objectOutput = true;

        RR_RETURN_ON_FAIL(compiler_->Compile(compileRequest, result.put()));

        for (size_t index = 0; index < result->GetOutputsCount(); index++)
        {
            Common::ComPtr<Rfx::IBlob> output;
            Rfx::CompileOutputType outputType;

            RR_RETURN_ON_FAIL(result->GetOutput(index, outputType, output.put()));

            if (outputType == Rfx::CompileOutputType::Assembly)
            {
                auto outputPath = assetPath;
                outputPath.replace_extension(".asm");
                RR_RETURN_ON_FAIL(SaveBlob(outputPath, output));

                ProcessorOutput processorOutput;
                processorOutput.path = outputPath;
                processorOutput.type = ProcessorOutput::Type::Output;

                outputs.emplace_back(processorOutput);
            }
        }

        LOG_INFO("Process ", asset.path.c_str());

        return Common::RResult::Ok;
    }
}