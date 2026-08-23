#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include "../job/threadPool.hpp"
#include "../rendering/types.hpp"
#include "../rendering/texture.hpp"

#define RESOURCE_MANAGER ResourceManager::instance()

class ResourceManager {
public:
	ResourceManager(const ResourceManager&) = delete;

	ResourceManager& operator=(const ResourceManager&) = delete;

	static ResourceManager& instance();

	template<typename T, typename KeyType>
	[[nodiscard]]
	T* get(const KeyType& key) const;

	void asyncLoadModel(size_t entityID, std::string modelPath, std::string texturePath);

	template<typename T>
	void upload(size_t entityID, T& map);

	void uploadMeshesToGPU();

	void uploadMaterialsToGPU();

	void waitForAll() const;

private:
	ResourceManager() = default;

	~ResourceManager() = default;

	void loadModel(size_t entityID, std::string_view modelPath, std::string_view texturePath);

	struct MaterialLoadContext;
	struct TextureLoadRequest;

	static void processMeshes(const aiNode* node,
	                          const aiScene* scene,
	                          MeshMap& meshesByMatID,
	                          MaterialLoadContext& materialLoadCtx);

	static Mesh processMesh(aiMesh* mesh);

	static void processMaterials(const aiScene* scene, MaterialLoadContext& materialLoadCtx);

	static void loadMaterialTextures(const aiMaterial* mat, const TextureLoadRequest& req, MaterialLoadContext& materialLoadCtx);

	static TextureType fromAssimpToTextureType(aiTextureType type);

	static aiTextureType fromTextureTypeToAssimp(TextureType type);

	struct MaterialLoadContext {
		MaterialMap materials;
		std::string textureDir;
		std::string roughMetalPath;
		std::unordered_set<uint32_t> materialsToLoad;
	};

	struct TextureLoadRequest {
		TextureType type;
		uint32_t materialID;
	};

	std::unordered_map<size_t, MaterialMap> mMaterialsByEntity;
	std::unordered_map<size_t, MeshMap> mMeshesByEntity;
	std::unordered_map<size_t, std::vector<float> > mTransformsByEntity;

	ThreadPool mThreadPool{};
	std::mutex mResourceMutex;

	static constexpr TextureType textureTypes[] = {
		TextureType::Albedo,
		TextureType::Normal,
		TextureType::Roughness_Metallic,
		TextureType::Ao,
		TextureType::Emissive,
		TextureType::Height
	};
};

#include "resourceManager.tpp"