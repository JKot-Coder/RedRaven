#pragma once

#include <memory>

//#include "render/shader.hpp"

namespace ResourceManager{

    class ResourceManager {
    public:
        inline static const std::unique_ptr<ResourceManager>& Instance() {
            return instance;
        }

       // const std::shared_ptr<Shader>& LoadShader();

    private:
        static std::unique_ptr<ResourceManager> instance;
    };

    inline static const std::unique_ptr<ResourceManager>& Instance() {
        return ResourceManager::Instance();
    }

}


