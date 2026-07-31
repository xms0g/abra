#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include "../job/threadPool.h"
#include "../rendering/types.hpp"

#define RESOURCE_MANAGER ResourceManager::instance()

class Shader;
class FrameBuffer;

class ResourceManager {
public:
	ResourceManager(const ResourceManager&) = delete;

	ResourceManager& operator=(const ResourceManager&) = delete;

	static ResourceManager& instance();

	template<typename T, typename KeyType>
	[[nodiscard]]
	T* get(const KeyType& key) const;

	void asyncLoadModel(size_t entityID, const std::string& file);

	template<typename T>
	void upload(size_t entityID, T& map);

	void uploadMeshesToGPU();

	void uploadMaterialsToGPU();

	void waitForAll() const;

private:
	ResourceManager() = default;

	~ResourceManager() = default;

	struct MaterialLoadContext {
		MaterialMap materials;
		std::string baseDir;
		std::string roughMetalPath;
		std::unordered_set<uint32_t> materialsToLoad;
	};

	struct TextureLoadRequest {
		const aiMaterial* mat;
		aiTextureType type;
		uint32_t materialID;
	};

	void loadModel(size_t entityID, const std::string& file);

	static void processMeshes(
		const aiNode* node,
		const aiScene* scene,
		MeshMap& meshesByMatID,
		MaterialLoadContext& materialLoadCtx);

	static Mesh processMesh(aiMesh* mesh);

	static void processMaterials(const aiScene* scene, MaterialLoadContext& materialLoadCtx);

	static void loadMaterialTextures(const TextureLoadRequest& req, MaterialLoadContext& materialLoadCtx);

	std::unordered_map<size_t, MaterialMap> mMaterialsByEntity;
	std::unordered_map<size_t, MeshMap> mMeshesByEntity;
	std::unordered_map<size_t, std::vector<float> > mTransformsByEntity;

	ThreadPool mThreadPool{};
	std::mutex mResourceMutex;

	static constexpr aiTextureType textureTypes[] = {
		aiTextureType_DIFFUSE,
		aiTextureType_NORMALS,
		aiTextureType_UNKNOWN,
		aiTextureType_LIGHTMAP,
		aiTextureType_EMISSIVE,
		aiTextureType_HEIGHT
	};
};

template<typename T, typename KeyType>
T* ResourceManager::get(const KeyType& key) const {
	if constexpr (std::is_same_v<T, MeshMap>) {
		return const_cast<T*>(&mMeshesByEntity.at(key));
	} else if constexpr (std::is_same_v<T, MaterialMap>) {
		return const_cast<T*>(&mMaterialsByEntity.at(key));
	} else if constexpr (std::is_same_v<T, std::vector<float>>) {
		return const_cast<T*>(&mTransformsByEntity.at(key));
	} else {
		static_assert(false, "Unsupported type for get().");
	}

	return nullptr;
}

template<typename T>
void ResourceManager::upload(size_t entityID, T& map) {
	if constexpr (std::is_same_v<T, MeshMap>) {
		mMeshesByEntity.emplace(entityID, std::move(map));
	} else if constexpr (std::is_same_v<T, MaterialMap>) {
		mMaterialsByEntity.emplace(entityID, std::move(map));
	} else if constexpr (std::is_same_v<T, std::vector<float>>) {
		mTransformsByEntity.emplace(entityID, std::move(map));
	}
}


