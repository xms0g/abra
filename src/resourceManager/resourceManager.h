#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include "../job/threadPool.h"
#include "../rendering/types.hpp"

class Shader;

class ResourceManager {
public:
	ResourceManager(const ResourceManager&) = delete;

	ResourceManager& operator=(const ResourceManager&) = delete;

	static ResourceManager& instance();

	[[nodiscard]]
	MeshMap* getMeshes(size_t entityID);

	[[nodiscard]]
	MaterialMap* getMaterial(size_t entityID);

	void asyncLoadModel(size_t entityID, std::string& file);

	std::vector<float>* uploadTransforms(size_t entityID, const std::vector<float>& transforms);

	void uploadModelsToGPU();

	void waitForAll() const;

private:
	explicit ResourceManager() = default;

	~ResourceManager() = default;

	struct MaterialLoadContext {
		MaterialMap materials;
		std::vector<uint32_t> materialsToLoad;
		std::string baseDir;
		std::string roughMetalPath;
	};

	struct TextureLoadRequest {
		const aiMaterial* mat;
		aiTextureType type;
		uint32_t materialID;
	};

	void loadModel(size_t entityID, const char* file);

	void processMeshes(
		const aiNode* node,
		const aiScene* scene,
		MeshMap& meshesByMatID,
		MaterialLoadContext& materialLoadCtx);

	Mesh processMesh(aiMesh* mesh) const;

	void processMaterials(const aiScene* scene, MaterialLoadContext& materialLoadCtx) const;

	void loadMaterialTextures(const TextureLoadRequest& req, MaterialLoadContext& materialLoadCtx) const;

	std::unordered_map<size_t, MaterialMap> mMaterialsByEntity;
	std::unordered_map<size_t, MeshMap> mMeshesByEntity;
	std::unordered_map<size_t, std::vector<float>> mTransformsByEntity;
	ThreadPool mThreadPool{};
	std::mutex mResourceMutex;

	static constexpr aiTextureType textureBindings[] = {
		aiTextureType_DIFFUSE,
		aiTextureType_NORMALS,
		aiTextureType_UNKNOWN,
		aiTextureType_LIGHTMAP,
		aiTextureType_EMISSIVE,
		aiTextureType_HEIGHT
	};
};
