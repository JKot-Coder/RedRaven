#include <vector>
#include <iostream>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "common/VecMath.h"

#include "rendering/Render.hpp"

#include "resource_manager/ResourcesLoaders.hpp"

using namespace Common;

namespace ResourceManager {

    vec3 ConvertVector(aiVector3D vector) {
        return vec3(vector.x, vector.y, vector.z);
    }

    std::vector<std::shared_ptr<Rendering::Mesh>> ResourcesLoaders::LoadScene(const std::shared_ptr<Common::Stream> &stream) {
        size_t dataSize = stream->GetSize();
        char *buffer = new char[dataSize];
        stream->Read(buffer, dataSize);

        auto scene = aiImportFileFromMemory(buffer, dataSize, aiProcessPreset_TargetRealtime_MaxQuality, stream->GetName().c_str());
        delete[] buffer;

        std::vector<std::shared_ptr<Rendering::Mesh>> renderMeshes;

        const auto meshes = scene->mMeshes;
        for(size_t i = 0; i < scene->mNumMeshes; i++){
            const auto mesh = meshes[i];

            std::vector<int32_t> indexes;
            for(size_t j = 0; j < mesh->mNumFaces; j++){
                indexes.push_back(mesh->mFaces[j].mIndices[0]);
                indexes.push_back(mesh->mFaces[j].mIndices[1]);
                indexes.push_back(mesh->mFaces[j].mIndices[2]);
            }

            std::vector<Rendering::Vertex> vertices;
            for(size_t j = 0; j < mesh->mNumVertices; j++) {
                Rendering::Vertex vertex;

                vertex.position = ConvertVector(mesh->mVertices[j]);
                vertex.normal = ConvertVector(mesh->mNormals[j]);

                vertices.push_back(vertex);
            }

            auto *render = Rendering::Instance().get();
            auto renderMesh = render->CreateMesh();
            renderMesh->Init(vertices, indexes);

            renderMeshes.push_back(renderMesh);
        }

        return renderMeshes;
    }
}