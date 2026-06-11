#include "import.h"

#include <filesystem>
#include <iostream>
#include <algorithm>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace fs = std::filesystem;

static bool extensionSupported(const fs::path& path)
{
    if (!path.has_extension())
        return false;

    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
    static const std::vector<std::string> supported = {".obj", ".fbx", ".dae", ".3ds", ".ply", ".glb", ".gltf"};
    return std::find(supported.begin(), supported.end(), ext) != supported.end();
}

static sf::Color materialColor(const aiScene* scene, const aiMesh* mesh)
{
    if (!scene || scene->mNumMaterials == 0 || mesh->mMaterialIndex >= scene->mNumMaterials)
        return sf::Color::White;

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    aiColor4D color;
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        return sf::Color(
            static_cast<uint8_t>(std::clamp(color.r, 0.0f, 1.0f) * 255.0f),
            static_cast<uint8_t>(std::clamp(color.g, 0.0f, 1.0f) * 255.0f),
            static_cast<uint8_t>(std::clamp(color.b, 0.0f, 1.0f) * 255.0f)
        );
    }
    return sf::Color::White;
}

static std::vector<std::unique_ptr<Triangle>> trianglesFromAiMesh(const aiMesh* mesh, const sf::Color& color)
{
    std::vector<std::unique_ptr<Triangle>> triangles;
    triangles.reserve(mesh->mNumFaces);

    for (unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
        const aiFace& face = mesh->mFaces[faceIndex];
        if (face.mNumIndices != 3)
            continue;

        const aiVector3D& a = mesh->mVertices[face.mIndices[0]];
        const aiVector3D& b = mesh->mVertices[face.mIndices[1]];
        const aiVector3D& c = mesh->mVertices[face.mIndices[2]];

        triangles.push_back(std::make_unique<Triangle>(
            Vector3f{a.x, a.y, a.z},
            Vector3f{b.x, b.y, b.z},
            Vector3f{c.x, c.y, c.z},
            color
        ));
    }

    return triangles;
}

static void importFile(const fs::path& path, std::vector<Mesh>& meshes)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.string(),
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenNormals |
        aiProcess_ImproveCacheLocality |
        aiProcess_OptimizeMeshes
    );

    if (!scene || !scene->HasMeshes()) {
        std::cerr << "Assimp failed to load " << path.string() << ": " << importer.GetErrorString() << "\n";
        return;
    }

    for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        const aiMesh* aiMesh = scene->mMeshes[meshIndex];
        sf::Color color = materialColor(scene, aiMesh);
        auto triangles = trianglesFromAiMesh(aiMesh, color);
        if (triangles.empty())
            continue;

        Mesh mesh(std::move(triangles), color);
        mesh.shadeSmooth = false;
        meshes.push_back(std::move(mesh));
    }
}

std::vector<Mesh> importMeshesFromFolder(const std::string& folderPath)
{
    std::vector<Mesh> meshes;
    fs::path folder(folderPath);

    if (!fs::exists(folder) || !fs::is_directory(folder)) {
        std::cerr << "Import folder not found: " << folderPath << "\n";
        return meshes;
    }

    for (const auto& entry : fs::directory_iterator(folder)) {
        if (!entry.is_regular_file())
            continue;
        if (!extensionSupported(entry.path()))
            continue;
        importFile(entry.path(), meshes);
    }

    return meshes;
}

