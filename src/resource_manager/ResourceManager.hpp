#pragma once

#include <string>
#include <memory>

namespace Rendering{
    class Shader;
    class Mesh;
    class CommonTexture;
    struct RenderElement;
}

namespace ResourceManager{

    class ResourceManager {
    public:
        inline static const std::unique_ptr<ResourceManager>& Instance() {
            return instance;
        }

        const std::shared_ptr<Rendering::Shader> LoadShader(const std::string& filename);
        const std::vector<Rendering::RenderElement> LoadScene(const std::string& filename);
        const std::shared_ptr<Rendering::CommonTexture> LoadTexture(const std::string& filename);
    private:
        static std::unique_ptr<ResourceManager> instance;
    };

    inline static const std::unique_ptr<ResourceManager>& Instance() {
        return ResourceManager::Instance();
    }

}


