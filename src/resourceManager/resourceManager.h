#pragma once
#include <string>
#include <vector>
#include <span>
#include <unordered_set>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../job/threadPool.h"

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

	void asyncLoadModel(size_t entityID, const char* file);

	void uploadModelsToGPU();

	void waitForAll() const;

private:
	explicit ResourceManager() = default;

	~ResourceManager() = default;

	void loadModel(size_t entityID, const char* file);

	void processNode(const aiNode* node, const aiScene* scene,
		MeshMap& meshesByMatID, MaterialMap& materials, std::unordered_set<std::string>& texturesLoaded);

	std::pair<uint32_t, Mesh> processMesh(aiMesh* mesh, const aiScene* scene, MaterialMap& materials,
		std::unordered_set<std::string>& texturesLoaded) const;

	void loadMaterialTextures(const aiMaterial* mat,
	                          aiTextureType type,
	                          const std::string& typeName,
	                          uint32_t materialID,
	                          MaterialMap& materials,
	                          std::unordered_set<std::string>& texturesLoaded) const;

	std::string mDirectory;
	std::unordered_map<size_t, MaterialMap> mMaterialsByEntity;
	std::unordered_map<size_t, MeshMap> mMeshesByEntity;
	ThreadPool mThreadPool{};
	std::mutex mResourceMutex;

	static constexpr const char* skyboxFaces[] = {
		"skybox/right.jpg",
		"skybox/left.jpg",
		"skybox/top.jpg",
		"skybox/bottom.jpg",
		"skybox/front.jpg",
		"skybox/back.jpg"
	};
};
