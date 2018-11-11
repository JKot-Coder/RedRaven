#include "ResourceManager.hpp"

namespace ResourceManager {

    std::unique_ptr<ResourceManager> ResourceManager::instance = std::unique_ptr<ResourceManager>(new ResourceManager());



}