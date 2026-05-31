#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "../job/threadPool.h"
#include "../rendering/types.hpp"

class Shader;
class BaseFrameBuffer;

class ResourceManager {
public:
	ResourceManager(const ResourceManager&) = delete;

	ResourceManager& operator=(const ResourceManager&) = delete;

	static ResourceManager& instance();

	void createShaders();

	void createBuffers();

	[[nodiscard]]
	MeshMap* getMeshes(size_t entityID);

	[[nodiscard]]
	MaterialMap* getMaterial(size_t entityID);

	[[nodiscard]]
	BaseFrameBuffer* getBuffer(const std::string& name) const;

	[[nodiscard]]
	Shader* getShader(const std::string& name) const;

	std::unordered_map<std::string, std::unique_ptr<Shader>>& getShaders();

	void asyncLoadModel(size_t entityID, std::string& file);

	void uploadMesh(size_t entityID, MeshMap& map);

	void uploadMaterial(size_t entityID, MaterialMap& map);

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

	uint32_t createEnvMap(const std::string& path);

	void createIrradianceMap();

	void createPrefilterMap();

	void createBrdfLUT();

	std::unordered_map<size_t, MaterialMap> mMaterialsByEntity;
	std::unordered_map<size_t, MeshMap> mMeshesByEntity;
	std::unordered_map<size_t, std::vector<float>> mTransformsByEntity;
	std::unordered_map<std::string, std::unique_ptr<BaseFrameBuffer>> mBuffers;
	std::unordered_map<std::string, std::unique_ptr<Shader>> mShaders;

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

	static constexpr uint32_t FACES = 6;

	glm::mat4 mCaptureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 mCaptureViews[FACES] = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
	};
};
