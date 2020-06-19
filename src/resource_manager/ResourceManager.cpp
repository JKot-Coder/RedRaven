#include "ResourceManager.hpp"

#include <vector>

#include "common/Common.hpp"
#include "common/Stream.hpp"
#include "common/VecMath.h"

#include "filesystem/FileSystem.hpp"

#include "rendering/Mesh.hpp"
#include "rendering/Render.hpp"
#include "rendering/Shader.hpp"

#include "resource_manager/ResourcesLoaders.hpp"

namespace OpenDemo
{
    namespace ResourceManager
    {
        std::unique_ptr<ResourceManager> ResourceManager::instance = std::unique_ptr<ResourceManager>(new ResourceManager());

        const std::shared_ptr<Rendering::Shader> ResourceManager::LoadShader(const U8String& filename)
        {
            auto* filesystem = FileSystem::Instance().get();

            const auto& render = Rendering::Instance().get();
            auto shader = render->CreateShader();

            std::shared_ptr<Common::Stream> stream;
            try
            {
                stream = filesystem->Open(filename, FileSystem::Mode::READ);
            }
            catch (const std::exception& exception)
            {
                Log::Fatal("Error opening resource \"%s\" with error: %s", filename.c_str(), exception.what());
                return shader;
            }

            shader->LinkSource(stream);
            return shader;
        }

        const std::vector<Rendering::RenderElement> ResourceManager::LoadScene(const U8String& filename)
        {
            //Todo: pass a stream for easy migrate to virtual file system in future
            return ResourcesLoaders::LoadScene(filename);
        }

        const std::shared_ptr<Rendering::CommonTexture> ResourceManager::LoadTexture(const U8String& filename)
        {
            auto* filesystem = FileSystem::Instance().get();

            std::shared_ptr<Common::Stream> stream;
            try
            {
                stream = filesystem->Open(filename, FileSystem::Mode::READ);
            }
            catch (const std::exception& exception)
            {
                Log::Error("Error opening resource \"%s\" with error: %s", filename.c_str(), exception.what());
            }

            return ResourcesLoaders::LoadTexture(stream);
        }
    }
}