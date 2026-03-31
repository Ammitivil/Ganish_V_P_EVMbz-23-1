#pragma once

#define GLEW_DLL
#define GLFW_DLL

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include <iostream>
#include "Mesh.h"
#include "Shaders.h"

class Model {
public:
    std::vector<Mesh> meshes;
    std::vector<glm::vec3> meshCenters; // центр каждого меша в локальных координатах
    std::string directory;

    Model(const char* path) {
        loadModel(path);
        computeMeshCenters();
    }

    void Draw(Shader& shader) {
        for (unsigned int i = 0; i < meshes.size(); i++) {
            meshes[i].Draw(shader);
        }
    }

    // Получить центр меша по индексу
    glm::vec3 getMeshCenter(int index) const {
        if (index >= 0 && index < (int)meshCenters.size())
            return meshCenters[index];
        return glm::vec3(0.0f);
    }

private:
    void loadModel(const std::string& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path,
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "Ошибка ASSIMP: " << importer.GetErrorString() << std::endl;
            return;
        }

        directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            if (mesh->HasNormals()) {
                vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            }
            else {
                vertex.Normal = glm::vec3(0.0f);
            }
            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        return Mesh(vertices, indices);
    }

    // Вычисление центров мешей (среднее арифметическое всех вершин)
    void computeMeshCenters() {
        meshCenters.resize(meshes.size());
        for (size_t i = 0; i < meshes.size(); ++i) {
            glm::vec3 center(0.0f);
            const auto& vertices = meshes[i].vertices;
            if (vertices.empty()) continue;
            for (const auto& v : vertices) {
                center += v.Position;
            }
            center /= (float)vertices.size();
            meshCenters[i] = center;
        }
    }
};