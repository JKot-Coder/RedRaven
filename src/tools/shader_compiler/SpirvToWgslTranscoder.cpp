#include "SpirvToWgslTranscoder.hpp"

#include "SubprocessRunner.hpp"
#include "common/OnScopeExit.hpp"
#include "common/Result.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace RR
{
    Common::RResult SpirvToWgslTranscoder::Transcode(slang::IBlob* spirvCode, std::string& wgslCode)
    {
        if (!spirvCode)
        {
            std::cerr << "SpirvToWgslTranscoder::Transcode: spirvCode is null" << std::endl;
            return Common::RResult::Fail;
        }

        std::filesystem::path tempDir = std::filesystem::temp_directory_path();
        std::filesystem::path tempSpvFile = tempDir / "shader_temp.spv";
        std::filesystem::path tempWgslFile = tempDir / "shader_temp.wgsl";

        {
            std::ofstream spvFile(tempSpvFile, std::ios::binary);
            if (!spvFile.is_open())
            {
                std::cerr << "SpirvToWgslTranscoder::Transcode: Failed to create temporary SPIR-V file: " << tempSpvFile << std::endl;
                return Common::RResult::Fail;
            }

            const void* spirvData = spirvCode->getBufferPointer();
            size_t spirvSize = spirvCode->getBufferSize();
            spvFile.write(static_cast<const char*>(spirvData), spirvSize);

            if (!spvFile.good())
            {
                std::cerr << "SpirvToWgslTranscoder::Transcode: Failed to write SPIR-V data to temporary file" << std::endl;
                spvFile.close();
                std::filesystem::remove(tempSpvFile);
                return Common::RResult::Fail;
            }

            spvFile.close();
        }

        ON_SCOPE_EXIT([&tempSpvFile, &tempWgslFile]() {
            if (std::filesystem::exists(tempSpvFile))
                std::filesystem::remove(tempSpvFile);
            if (std::filesystem::exists(tempWgslFile))
                std::filesystem::remove(tempWgslFile);
        });

        std::string spvFilePath = tempSpvFile.string();
        std::string wgslFilePath = tempWgslFile.string();

        std::vector<const char*> args = {
            "./tint",
            "--format", "wgsl",
            "-o", wgslFilePath.c_str(),
            spvFilePath.c_str(),
            nullptr
        };

        SubprocessResult processResult;
        if (RR_FAILED(SubprocessRunner::Run(args, processResult)))
        {
            std::cerr << "SpirvToWgslTranscoder::Transcode: Tint process failed" << std::endl;
            return Common::RResult::Fail;
        }

        if (processResult.exitCode != 0)
        {
            std::cerr << "SpirvToWgslTranscoder::Transcode: Tint process failed with exit code: " << processResult.exitCode << std::endl;
            if (!processResult.output.empty())
            {
                std::cerr << "Tint error output:\n" << processResult.output << std::endl;
            }
            return Common::RResult::Fail;
        }

        if (!std::filesystem::exists(tempWgslFile))
        {
            std::cerr << "SpirvToWgslTranscoder::Transcode: Tint did not create output WGSL file: " << tempWgslFile << std::endl;
            if (!processResult.output.empty())
            {
                std::cerr << "Tint output:\n" << processResult.output << std::endl;
            }
            return Common::RResult::Fail;
        }

        {
            std::ifstream wgslFile(tempWgslFile);
            if (!wgslFile.is_open())
            {
                std::cerr << "SpirvToWgslTranscoder::Transcode: Failed to open generated WGSL file: " << tempWgslFile << std::endl;
                return Common::RResult::Fail;
            }

            wgslCode.assign(std::istreambuf_iterator<char>(wgslFile), std::istreambuf_iterator<char>());
            wgslFile.close();

            if (wgslCode.empty())
            {
                std::cerr << "SpirvToWgslTranscoder::Transcode: Generated WGSL code is empty" << std::endl;
                return Common::RResult::Fail;
            }
        }

        return Common::RResult::Ok;
    }
}

