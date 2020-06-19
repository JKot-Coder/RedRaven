#include "ResourcesLoaders.hpp"

#include <iostream>
#include <vector>

#include <stb_image.h>

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "common/Stream.hpp"
#include "common/VecMath.h"
#include "filesystem/FileSystem.hpp"

#include "rendering/Material.hpp"
#include "rendering/Mesh.hpp"
#include "rendering/Render.hpp"
#include "rendering/Texture.hpp"

#include "resource_manager/ResourceManager.hpp"
#include "resource_manager/ResourcesLoaders.hpp"

namespace OpenDemo
{
    using namespace Common;

    namespace ResourceManager
    {

        const vec3 ConvertVector(const aiVector3D& vector)
        {
            return vec3(vector.x, vector.y, vector.z);
        }

        const vec2 ConvertVector(const aiVector2D& vector)
        {
            return vec2(vector.x, vector.y);
        }

        const std::shared_ptr<Rendering::CommonTexture> LoadMaterialTexture(const U8String& path, const aiMaterial* material, aiTextureType type)
        {
            const auto& resourceManager = ResourceManager::Instance().get();
            if (material->GetTextureCount(type) == 0)
                return nullptr;

            aiString textureName("");
            aiReturn ok = material->GetTexture(type, 0, &textureName);

            if (ok != aiReturn_SUCCESS)
                return nullptr;

            //TODO: use wstring on windows platform to avoid lost of non latin characters
            return resourceManager->LoadTexture((fs::path(path) / textureName.C_Str()).string());
        }

        const std::vector<Rendering::RenderElement> ResourcesLoaders::LoadScene(const U8String& filename)
        {
            //Todo: Replace to aiImportFileEx, it's needed for implementing virtual file system
            auto scene = aiImportFile(filename.data(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_FlipUVs);
            std::vector<Rendering::RenderElement> renderElements;
            std::vector<Rendering::Material> renderMaterials;

            const auto& materials = scene->mMaterials;
            for (size_t i = 0; i < scene->mNumMaterials; i++)
            {
                const auto& material = materials[i];
                Rendering::Material renderingMaterial;

                const auto& directoryPath = fs::path(filename).parent_path();

                renderingMaterial.albedoMap = LoadMaterialTexture(directoryPath, material, aiTextureType_DIFFUSE);
                renderingMaterial.normalMap = LoadMaterialTexture(directoryPath, material, aiTextureType_NORMALS);
                renderingMaterial.roughnessMap = LoadMaterialTexture(directoryPath, material, aiTextureType_SHININESS);
                renderingMaterial.metallicMap = LoadMaterialTexture(directoryPath, material, aiTextureType_AMBIENT);

                renderMaterials.push_back(renderingMaterial);
            }

            const auto& meshes = scene->mMeshes;
            for (size_t i = 0; i < scene->mNumMeshes; i++)
            {
                const auto& mesh = meshes[i];

                std::vector<int32_t> indexes;
                for (size_t j = 0; j < mesh->mNumFaces; j++)
                {
                    indexes.push_back(mesh->mFaces[j].mIndices[0]);
                    indexes.push_back(mesh->mFaces[j].mIndices[1]);
                    indexes.push_back(mesh->mFaces[j].mIndices[2]);
                }

                std::vector<Rendering::Vertex> vertices;
                for (size_t j = 0; j < mesh->mNumVertices; j++)
                {
                    Rendering::Vertex vertex;

                    vec3 texCoord(0, 0, 0);

                    if (mesh->mNumUVComponents[0] == 2)
                        texCoord = ConvertVector(mesh->mTextureCoords[0][j]);

                    vertex.position = ConvertVector(mesh->mVertices[j]);
                    vertex.normal = ConvertVector(mesh->mNormals[j]);
                    vertex.tangent = ConvertVector(mesh->mTangents[j]);
                    vertex.binormal = ConvertVector(mesh->mBitangents[j]);
                    vertex.texCoord = vec2(texCoord.x, texCoord.y);

                    vertices.push_back(vertex);
                }

                auto* render = Rendering::Instance().get();
                auto renderMesh = render->CreateMesh();
                renderMesh->Init(vertices, indexes);

                Rendering::RenderElement renderElement;
                renderElement.mesh = renderMesh;
                renderElement.modelMatrix.identity();
                renderElement.material = renderMaterials[mesh->mMaterialIndex];

                renderElements.push_back(renderElement);
            }

            return renderElements;
        }

        const std::shared_ptr<Rendering::CommonTexture> ResourcesLoaders::LoadTexture(const std::shared_ptr<Common::Stream>& stream)
        {
            const auto& render = Rendering::Instance().get();

            size_t dataSize = static_cast<size_t>(stream->GetSize());
            char* buffer = new char[dataSize];
            stream->Read(buffer, dataSize);

            int width, height, channels;
            const auto image = stbi_load_from_memory(reinterpret_cast<uint8_t*>(buffer), static_cast<int>(dataSize), &width, &height,
                &channels, STBI_default);

            delete[] buffer;

            auto texture = render->CreateTexture2D();

            if (image == nullptr)
            {
                return texture;
            }

            Rendering::Texture2D::Description textureDescription;
            textureDescription.height = width;
            textureDescription.width = height;

            switch (channels)
            {
            case 1:
                textureDescription.pixelFormat = Rendering::R8;
                break;
            case 2:
                textureDescription.pixelFormat = Rendering::RG8;
                break;
            case 3:
                textureDescription.pixelFormat = Rendering::RGB8;
                break;
            case 4:
                textureDescription.pixelFormat = Rendering::RGBA8;
                break;
            }

            texture->Init(textureDescription, image);
            stbi_image_free(image);
            return texture;
        }
    }
}