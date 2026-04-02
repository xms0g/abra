#pragma once
#include <string>
#include <vector>
#include <span>
#include <unordered_set>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../job/threadPool.h"
#include "../rendering/types.hpp"

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

	void processMeshes(const aiNode* node, const aiScene* scene, MeshMap& meshesByMatID);

	Mesh processMesh(aiMesh* mesh) const;

	struct MaterialLoadContext {
		MaterialMap materials;
		std::unordered_set<std::string> texturesLoaded;
		std::string baseDir;
	};

	struct TextureLoadRequest {
		const aiMaterial* mat;
		aiTextureType type;
		std::string_view typeName;
		uint32_t materialID;
	};

	void processMaterials(const aiScene* scene, MaterialLoadContext& ctx) const;

	void loadMaterialTextures(const TextureLoadRequest& req, MaterialLoadContext& ctx) const;

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

	struct TextureBinding {
		aiTextureType type;
		const char* name;
	};

	static constexpr TextureBinding textureBindings[] = {
		{aiTextureType_METALNESS, "texture_metallic"},
		{aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness"},
		{aiTextureType_DIFFUSE, "texture_albedo"},
		{aiTextureType_SPECULAR, "texture_specular"},
		{aiTextureType_NORMALS, "texture_normal"},
		{aiTextureType_HEIGHT, "texture_height"},
		{aiTextureType_EMISSIVE, "texture_emissive"},
		{aiTextureType_LIGHTMAP,"texture_ao"},
	};
};
