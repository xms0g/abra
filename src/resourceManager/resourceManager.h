#pragma once
#include <string>
#include <vector>
#include <span>
#include <unordered_set>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Material;
using MaterialMap = std::unordered_map<uint32_t, Material >;
class Mesh;
using MeshMap = std::unordered_map<uint32_t, std::vector<Mesh> >;

class Shader;

class ResourceManager {
public:
	ResourceManager(const ResourceManager&) = delete;

	ResourceManager& operator=(const ResourceManager&) = delete;

	static ResourceManager& instance();

	[[nodiscard]] MeshMap* getMeshes(size_t entityID);

	[[nodiscard]] const MaterialMap* getMaterial(size_t entityID) const;

	[[nodiscard]] std::span<const char* const> getSkyboxTexture() const;

	void loadModel(size_t entityID, const char* file);

private:
	explicit ResourceManager() = default;

	~ResourceManager() = default;

	void processNode(const aiNode* node, const aiScene* scene);

	std::pair<uint32_t, Mesh> processMesh(aiMesh* mesh, const aiScene* scene);

	void loadMaterialTextures(const aiMaterial* mat,
	                          aiTextureType type,
	                          const std::string& typeName,
	                          uint32_t materialID);

	std::string mDirectory;
	MaterialMap mMaterials;
	MeshMap mMeshesByMatID;
	std::unordered_map<size_t, MaterialMap> mMaterialsByEntity;
	std::unordered_map<size_t, MeshMap> mMeshesByEntity;
	std::unordered_set<std::string> mTexturesLoaded;

	static constexpr const char* skyboxFaces[] = {
		"skybox/right.jpg",
		"skybox/left.jpg",
		"skybox/top.jpg",
		"skybox/bottom.jpg",
		"skybox/front.jpg",
		"skybox/back.jpg"
	};
};
